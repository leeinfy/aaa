/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <zephyr/kernel.h>
//#include <zephyr/logging/log.h>
#include <sys/printk.h>
#include <dk_buttons_and_leds.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gap.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

#include <bluetooth/services/nus.h>
#include <bluetooth/services/throughput.h>


#define DEVICE_NAME				CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN			(sizeof(DEVICE_NAME)-1)

#define NOTIFY_INTERVAL 500

#define STACKSIZE 2048
#define PRIORITY	7

#define BT_UUID_SERVER_VAL\
	BT_UUID_128_ENCODE(0x00001523,0x1212,0xefde,0x1523,0x785feabcd123)
#define BT_UUID_BUTTON_VAL\
	BT_UUID_128_ENCODE(0x00001524,0x1212,0xefde,0x1523,0x785feabcd123)
#define BT_UUID_LED_VAL\
	BT_UUID_128_ENCODE(0x00001525,0x1212,0xefde,0x1523,0x785feabcd123)
#define BT_UUID_CUSTOM_SENSOR_VAL\
	BT_UUID_128_ENCODE(0x00001526,0x1212,0xefde,0x1523,0x785feabcd123)


#define BT_UUID_SERVER		BT_UUID_DECLARE_128(BT_UUID_SERVER_VAL)
#define BT_UUID_BUTTON		BT_UUID_DECLARE_128(BT_UUID_BUTTON_VAL)
#define BT_UUID_LED			BT_UUID_DECLARE_128(BT_UUID_LED_VAL)
#define BT_UUID_CUSTOM_SENSOR		BT_UUID_DECLARE_128(BT_UUID_CUSTOM_SENSOR_VAL)

static const struct bt_le_adv_param* adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_USE_IDENTITY),
																200,
																201,
																NULL);

static const struct bt_data ad[] ={
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

struct bt_conn *client = NULL;

static uint8_t button_state = 0;

static ssize_t attr_read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
						   uint16_t len, uint16_t offset){
	printk("Attribut read, handle: %u, conn %p\n", attr->handle, (void *)conn);
	const char *value = attr->user_data;
	button_state = button_state^1;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
}

static ssize_t attr_write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
						   uint16_t len, uint16_t offset, uint8_t flags){
	printk("Attribut write, handle: %u, conn %p\n", attr->handle, (void *)conn);
	if (len != 1U){
		printk ("write led: incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}
	uint8_t val = *((uint8_t*)buf);
	if(val == 1) dk_set_led_on(DK_LED2);
	else dk_set_led_off(DK_LED2);

	return len;
}

static bool bt_mysensor_notify_enabled;

static void bt_custom_sesnor_cfg_cb(const struct bt_gatt_attr *attr, uint16_t value){
	bt_mysensor_notify_enabled = (value == BT_GATT_CCC_NOTIFY);
	printk("BLE sensor notify status: %d\n", bt_mysensor_notify_enabled);
}

static uint8_t sensor_value = 64;

BT_GATT_SERVICE_DEFINE(custom_server,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_SERVER),
	BT_GATT_CHARACTERISTIC(BT_UUID_BUTTON,
						   BT_GATT_CHRC_READ,
						   BT_GATT_PERM_READ,
						   attr_read_button, NULL, &button_state),

	BT_GATT_CHARACTERISTIC(BT_UUID_LED,
						   BT_GATT_CHRC_WRITE,
						   BT_GATT_PERM_WRITE,
						   NULL, attr_write_led, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_CUSTOM_SENSOR,
						   BT_GATT_CHRC_NOTIFY,
						   BT_GATT_PERM_NONE,
						   NULL, NULL, NULL),
	
	BT_GATT_CCC(bt_custom_sesnor_cfg_cb, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

int bt_send_sensor_notify(uint32_t sensor_value){
	if (!bt_mysensor_notify_enabled) return -EACCES;
	return bt_gatt_notify(NULL, &custom_server.attrs[6], &sensor_value, sizeof(sensor_value));
}

static void sim_sensor_data(){
	sensor_value++;
	if(sensor_value == 91){
		sensor_value = 65;
	}
}

/*
static void button_handler(uint32_t button_state, uint32_t has_changed){
	if (has_changed & DK_BTN1_MSK) {
		printk("button1 press\n");
	}
}
*/

static void bt_connect_cb(struct bt_conn *conn, uint8_t err){
	if (err) {
		printk ("BT connection failed, err(%d)\n", err);
	}
	dk_set_led_on(DK_LED1);
	client = bt_conn_ref(conn);
}

static void bt_disconnect_cb (struct bt_conn *conn, uint8_t err){
	if (err) {
		printk ("BT disconnection failed, err(%d)\n", err);
	}
	dk_set_led_off(DK_LED1);
}

struct bt_conn_cb bt_connection_callbacks = {
	.connected = bt_connect_cb,
	.disconnected = bt_disconnect_cb
};

static bool nus_tx_enabled;

void bt_nus_send_enable_cb (enum bt_nus_send_status status){
	nus_tx_enabled = (status == BT_NUS_SEND_STATUS_ENABLED);
	printk("BLE NUS send status: %d\n", nus_tx_enabled);
	if (nus_tx_enabled == 1){
		printk("BLE NUS MTU (%d)\n", bt_nus_get_mtu(client));
	}
}

void bt_nus_rx_cb (struct bt_conn* conn, const uint8_t* const data, uint16_t len){
	for(int i=0;i<len;i++){
		printk("received: (%d)\n", data[i]);
	}
}

static struct bt_nus_cb nus_cb = {
	.send_enabled = bt_nus_send_enable_cb,
	.received = bt_nus_rx_cb
};

int bt_nus_send_notify(uint8_t sensor_value){
	if (!nus_tx_enabled) return -EACCES;
	return bt_nus_send(client, &sensor_value, sizeof(sensor_value));
}

void send_data_thread(void){
	while(1){
		sim_sensor_data();
		bt_nus_send_notify(sensor_value);
		k_sleep(K_MSEC(NOTIFY_INTERVAL));
	}
}

static struct bt_throughput throughput;
static struct bt_throughput_cb throughput_cb;

void main(void)
{
	int err;

	err = dk_leds_init();
	if (err){
		printk("LED init failed, err(%d)\n", err);
	}
	
	err = dk_buttons_init(NULL);
	if (err){
		printk("Button init failed, err(%d)\n", err);
	}

	err = bt_enable(NULL);
	if (err){
		printk("BT init failed, err(%d)\n", err);
	}

	bt_conn_cb_register(&bt_connection_callbacks);

	err = bt_nus_init(&nus_cb);
	if (err){
		printk("BT NUS init failed, err(%d)\n", err);
	}

	err = bt_throughput_init(&throughput, &throughput_cb);
	if (err) {
		printk("BT throughput init failed, err(%d)\n", err);
		return;
	}

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err){
		printk("BT advertisement failed, err(%d)\n", err);
		return;
	}
}

K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);