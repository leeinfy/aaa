/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <zephyr/kernel.h>
#include <drivers/gpio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gap.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/gatt.h>
#include <sys/printk.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)

static struct bt_gatt_exchange_params exchange_params;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL| BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

static const struct bt_le_adv_param* adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_USE_IDENTITY),
																800,
																801,
																NULL);

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

static void exchange_func(struct bt_conn *conn, uint8_t att_err, struct bt_gatt_exchange_params *params){
	if(!att_err){
		uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;
		printk("New MTU: %d", payload_mtu);
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

void on_connected(struct bt_conn *conn, uint8_t err){
	printk("connected\n");
	struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if(err){
		printk("bt_conn_get_info reutnred %d", err);
	return;
	}
	double connection_interval = info.le.interval*1.25;
	uint16_t supervision_timeout = info.le.timeout*10;
	printk("interval: %.2f ms, latency %d, timeout %d\n", connection_interval, info.le.latency, supervision_timeout);

	update_phy(conn);
	update_data_length(conn);
	update_mtu(conn);
}

void on_disconnected(struct bt_conn *conn, uint8_t err){
	printk("disconnected\n");
}

void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t time_out){
	double connection_interval = interval*1.25;
	uint16_t supervision_timeout = time_out*10;
	printk("change val: %.2f ms, latency %d, timeout %d\n", connection_interval, latency, supervision_timeout);
}

void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param){
	if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) printk("PHY update. 1M\n");
	else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) printk("PHY update. 2M\n");
	else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) printk("PHY update Long Range\n");
}

void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info){
	uint16_t tx_len = info->tx_max_len;
	uint16_t tx_time = info->tx_max_time;
	uint16_t rx_len = info->rx_max_len;
	uint16_t rx_time = info->rx_max_time;
	printk("tx_len %d, tx_time %d, rx_len %d, rx_time %d\n",tx_len,tx_time,rx_len,rx_time);
}

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
	.le_param_updated = on_le_param_updated,
	.le_phy_updated = on_le_phy_updated,
	.le_data_len_updated = on_le_data_len_updated
};


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void main(void)
{	
	int ret;

	if (!device_is_ready(led.port)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	int err;

	bt_addr_le_t addr;
	err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
	if (err){
		printk("Invalid BT address (err %d) \n",err);
		return;
	}

	err = bt_id_create(&addr, NULL);
	if (err){
		printk("Creating new ID fail (err %d) \n",err);
		return;
	}

	bt_conn_cb_register(&connection_callbacks);

	err = bt_enable(NULL);
	if (err){
		printk("Bluetooth init failed");
		return;
	}
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err){
		printk("Bluetooth init failed");
		return;
	}

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
	}
}
