#include "i2c.h"

//all api funciton would be able to find at
//https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.0/nrfx/drivers/twim/driver.html 

const nrfx_twim_t i2c1_dev = NRFX_TWIM_INSTANCE(1);
//the pin is listed in .dts file i2c1
const nrfx_twim_config_t i2c1_config = {.scl = 35,
										.sda = 34,
										.frequency = NRF_TWIM_FREQ_100K,
										.interrupt_priority = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY,
										.hold_bus_uninit = false,
};

void init_i2c1(){
	printk("init i2c1\n");
	nrfx_twim_uninit(&i2c1_dev);
    nrfx_err_t err;
    err = nrfx_twim_init(&i2c1_dev,&i2c1_config,NULL,NULL);
	if (err!=NRFX_SUCCESS) printk("i2c1 init fail\n");
    nrfx_twim_enable(&i2c1_dev);
}

nrfx_err_t i2c1_send(uint8_t addr, uint8_t *data, uint8_t length){
	nrfx_twim_xfer_desc_t i2c1_xfer_desc = NRFX_TWIM_XFER_DESC_TX(addr, data, length);
	return nrfx_twim_xfer(&i2c1_dev,&i2c1_xfer_desc,0);
}

nrfx_err_t i2c1_send_no_stop(uint8_t addr, uint8_t *data, uint8_t length){
	nrfx_twim_xfer_desc_t i2c1_xfer_desc = NRFX_TWIM_XFER_DESC_TX(addr, data, length);
	return nrfx_twim_xfer(&i2c1_dev,&i2c1_xfer_desc,NRFX_TWIM_FLAG_TX_NO_STOP);

}

nrfx_err_t i2c1_receive(uint8_t addr, uint8_t *data, uint8_t length){
	nrfx_twim_xfer_desc_t i2c1_xfer_desc = NRFX_TWIM_XFER_DESC_RX(addr, data, length);
	return nrfx_twim_xfer(&i2c1_dev,&i2c1_xfer_desc,0);
}

nrfx_err_t i2c1_send_follow_receive(uint8_t addr, uint8_t *tx_data, uint8_t tx_len, uint8_t* rx_data, uint8_t rx_len){
	nrfx_twim_xfer_desc_t i2c1_xfer_desc = NRFX_TWIM_XFER_DESC_TXRX(addr, tx_data, tx_len, rx_data, rx_len);
	return nrfx_twim_xfer(&i2c1_dev,&i2c1_xfer_desc,0);
}