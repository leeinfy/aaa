#ifndef I2C_H_
#define I2C_H_

#include <nrfx_twim.h>
#include <zephyr.h>
#include <sys/printk.h>

void init_i2c1();
nrfx_err_t i2c1_send(uint8_t addr, uint8_t *data, uint8_t length);
nrfx_err_t i2c1_send_no_stop(uint8_t addr, uint8_t *data, uint8_t length);
nrfx_err_t i2c1_receive(uint8_t addr, uint8_t *data, uint8_t length);
nrfx_err_t i2c1_send_follow_receive(uint8_t addr, uint8_t *tx_data, uint8_t tx_len, uint8_t* rx_data, uint8_t rx_len);

#endif