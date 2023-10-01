/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <zephyr/kernel.h>

K_FIFO_DEFINE(fifo_test);

struct data_type_t{
	void *fifo_reserved;
	uint8_t data_content;
};

struct data_type_t* tx_data = NULL;
struct data_type_t* rx_data = NULL;
int x=0;

void add_data_fifo(){
	tx_data = k_malloc(sizeof(*tx_data));
	tx_data->data_content = x;
	k_fifo_put(&fifo_test, tx_data);
	x++;
}

void main(void)
{
	k_sleep(K_SECONDS(1));
	for(int x=0;x<5;x++){
		add_data_fifo();
	}
	int x = 0;
	while(1){
		if(!k_fifo_is_empty(&fifo_test)){
			rx_data = k_fifo_get(&fifo_test,K_FOREVER);
			printk("%d ", x);
			printk("output: rx_data (%d)\n", rx_data->data_content);
			x++;
			k_free(rx_data);
		}
		k_sleep(K_MSEC(500));
	}
}
