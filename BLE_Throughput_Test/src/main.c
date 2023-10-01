/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <dk_buttons_and_leds.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gap.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#define DEVICE_NAME				CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN			(sizeof(DEVICE_NAME)-1)

struct bt_conn *phone_conn = NULL;
static bool nus_tx_enabled = false;
uint8_t mtu_size;
static struct bt_gatt_exchange_params exchange_params;

static const struct bt_le_adv_param* adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_USE_IDENTITY),
																200,
																201,
																NULL);

static const struct bt_data ad[] ={
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

static void exchange_func(struct bt_conn *conn, uint8_t att_err, struct bt_gatt_exchange_params *params){
	if(!att_err){
		uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;
	}
}

static void update_mtu(struct bt_conn *conn){
	int err;
	exchange_params.func = exchange_func;
	err = bt_gatt_exchange_mtu(conn, &exchange_params);
	if (err){
		printk("bt_gatt_exchange_mtu fail (err %d)", err);
	}
}

static void update_data_length(struct bt_conn *conn){
	int err;
	struct bt_conn_le_data_len_param my_data_len = {
		.tx_max_len = BT_GAP_DATA_LEN_MAX,
		.tx_max_time = BT_GAP_DATA_TIME_MAX
	};
	err = bt_conn_le_data_len_update(conn, &my_data_len);
	if(err){
		printk("data_len_update fail (err %d)\n", err);
	}
}

static void  update_phy(struct bt_conn *conn){
	int err;
	const struct bt_conn_le_phy_param preferred_phy ={
		.options = BT_CONN_LE_PHY_OPT_NONE,
		.pref_rx_phy = BT_GAP_LE_PHY_2M,
		.pref_tx_phy = BT_GAP_LE_PHY_2M
	};
	err = bt_conn_le_phy_update(conn, &preferred_phy);
	if (err){
		printk("bt_conn_le_phy_update() return %d", err);
	}
}

static void le_data_length_updated(struct bt_conn *conn,
				   struct bt_conn_le_data_len_info *info)
{
	printk("LE data len updated: TX (len: %d time: %d)"
	       " RX (len: %d time: %d)\n", info->tx_max_len,
	       info->tx_max_time, info->rx_max_len, info->rx_max_time);
	
}

static void bt_connect_cb(struct bt_conn *conn, uint8_t err){
	if (err) {
		printk ("BT connection failed, err(%d)\n", err);
	}
	dk_set_led_on(DK_LED1);
	phone_conn = bt_conn_ref(conn);

	update_phy(conn);
	update_data_length(conn);
	update_mtu(conn);
}

static void bt_disconnect_cb (struct bt_conn *conn, uint8_t err){
	if (err) {
		printk ("BT disconnection failed, err(%d)\n", err);
	}
	dk_set_led_off(DK_LED1);
}

struct bt_conn_cb bt_connection_callbacks = {
	.connected = bt_connect_cb,
	.disconnected = bt_disconnect_cb,
	.le_data_len_updated = le_data_length_updated
};

void bt_nus_send_enable_cb (enum bt_nus_send_status status){
	nus_tx_enabled = (status == BT_NUS_SEND_STATUS_ENABLED);
	printk("BLE NUS send status: %d\n", nus_tx_enabled);
}

static struct bt_nus_cb nus_cb = {
	.send_enabled = bt_nus_send_enable_cb,
};


static int shell_test_handler(const struct shell *sh, size_t argc, char **argv){
	int err;
	mtu_size = bt_nus_get_mtu(phone_conn);
	printk("BLE NUS MTU (%d)\n", mtu_size);
	uint8_t img[mtu_size];
	int send_data_size = 0;
	if(!nus_tx_enabled){
		printk("nus tx is not enabled by the central\n");
		return 0;
	}
	uint64_t stamp;
	int64_t delta;
	stamp = k_uptime_get();
	printk("starting a test run.........................\n");
	while(send_data_size < 10000000){
		err = bt_nus_send(phone_conn, (char*)img, (mtu_size));
		if (err) {
			printk("nus send failed (err %d)", err);
			break;
		}
	send_data_size += mtu_size;
	}
	delta = k_uptime_delta(&stamp);

	printk("send %d in %lld ms, speed %lldKBps\n",send_data_size, delta, send_data_size/delta);
	return 0;
}

//Create a shell cmd that start the test script
SHELL_CMD_REGISTER(test, NULL, "start a test run", &shell_test_handler);



void main(void){
	k_sleep(K_SECONDS(1));
	printk("This is a program for testing the throughput between NRF and PHONE\n");

	int err;

	err = dk_leds_init();
	if (err){
		printk("LED init failed, err(%d)\n", err);
	}

	err = bt_enable(NULL);
	if (err){
		printk("BT init failed, err(%d)\n", err);
	}

	//assign the callback for ble connection
	bt_conn_cb_register(&bt_connection_callbacks);

	//init the nordic ble uart service
	err = bt_nus_init(&nus_cb);
	if (err){
		printk("BT NUS init failed, err(%d)\n", err);
	}

	//start advertising
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err){
		printk("BT advertisement failed, err(%d)\n", err);
		return;
	}
}
