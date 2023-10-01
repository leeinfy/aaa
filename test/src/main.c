#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <nrfx_spim.h>

#define SPIM_SCLK         (37)      //p1.05
#define SPIM_MOSI         (47)
#define SPIM_MISO         (45)
#define SPIM_SS           (43)


// #define SPIM_SCLK         (8)
// #define SPIM_MOSI         (9)
// #define SPIM_MISO         (10)
// #define SPIM_SS           (11)

static const nrfx_spim_t spi_dev = NRFX_SPIM_INSTANCE(2);

void spi_init(void) {

    nrfx_err_t err;
    nrfx_spim_uninit(&spi_dev);
    nrfx_spim_config_t config = NRFX_SPIM_DEFAULT_CONFIG(SPIM_SCLK,SPIM_MOSI,SPIM_MISO,SPIM_SS);
    config.frequency = NRF_SPIM_FREQ_4M;
    //config.miso_pull = GPIO_PIN_CNF_PULL_Pulldown;
	err = nrfx_spim_init(&spi_dev, &config, NULL, NULL);
	if (err != NRFX_SUCCESS) {
		printk("Failed to initialize SPI: 0x%08X\n", err);
		return;
	}
}

void spi_send_receive(uint8_t * send_buf, size_t send_length, uint8_t * recv_buf, size_t recv_length) {
    int err = 0;

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(send_buf, send_length, recv_buf, recv_length);

    err = nrfx_spim_xfer(&spi_dev, &xfer_desc, 0);
	if (err != NRFX_SUCCESS) {
		printk("Transfer failed: 0x%08X\n", err);
	}
}

void main(void){
    k_msleep(5000);
    printk("start\n");
    spi_init();
    uint8_t intdata[2] = {0x03, 0x10};
    uint8_t tx_data[5] = {0x80, 0x83, 0x85, 0x86, 0x00};
    uint8_t rx_data[5] = {0x00};
    spi_send_receive(intdata, 2, NULL, 0);
    k_msleep(1000);
    while (1){ 
        spi_send_receive(tx_data, 5, rx_data, 5);
        printk("data: %x,%x,%x,%x,%x\n",rx_data[0],rx_data[1],rx_data[2],rx_data[3],rx_data[4]);
        // Wait
        k_msleep(1000);
    }
}
