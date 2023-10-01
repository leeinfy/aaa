#include <drivers/gpio.h>
#include <logging/log.h>
#include <nrfx_spim.h>

#define LOG_MODULE_NAME         nordic_spi_direct_c
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_ERR);

#include "spi.h"

#define SPI_NODE  DT_NODELABEL(spi4)

spi_priv_t spi_priv;

void spi_init(void) {

    spi_priv.spi = (nrfx_spim_t) NRFX_SPIM_INSTANCE(4);
    nrfx_err_t err;

    nrfx_spim_config_t config = NRFX_SPIM_DEFAULT_CONFIG(
		CONFIG_SPI_SCK_PIN,
        CONFIG_SPI_MOSI_PIN,
        CONFIG_SPI_MISO_PIN,
		NRF_DT_GPIOS_TO_PSEL(SPI_NODE, cs_gpios));
	config.frequency = NRF_SPIM_FREQ_4M;
	config.mode = NRF_SPIM_MODE_0;

	err = nrfx_spim_init(&spi_priv.spi, &config, NULL, NULL);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("Failed to initialize SPI: 0x%08X\n", err);
		return;
	}

    spi_priv.is_initialized = true;
}


bool spi_is_initialized(void) {

    return spi_priv.is_initialized;
}

int spi_send_receive(uint8_t * send_buf, size_t send_length, uint8_t * recv_buf, size_t recv_length) {
    int err = 0;

    nrfx_spim_xfer_desc_t xfer_desc = {
        .p_tx_buffer = send_buf,
        .tx_length = send_length,
        .p_rx_buffer = recv_buf,
        .rx_length = recv_length,
    };

    err = nrfx_spim_xfer(&spi_priv.spi, &xfer_desc, 0);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("Transfer failed: 0x%08X\n", err);
        return err;
	}
    else {
        return 0; //TODO: Fix this, it would be nice if upper level application is aware of NRFX error codes. But callers are now expecting 0.
    }
}

void spi_test_send(void) {
	uint8_t tx_buffer[4] = {0x80, 0x80, 0x80, 0x80};
	uint8_t rx_buffer[4] = {0xFF, 0xF0, 0x0F, 0x00};

    spi_send_receive(tx_buffer, 4, NULL, 0);
}

// nrfx_spim_t spi1_dev = NRFX_SPIM_INSTANCE(1);

// void spi1_init (void){
//     nrfx_spim_config_t spi1_config = NRFX_SPIM_DEFAULT_CONFIG(0,1,2,3);
//     nrfx_spim_uinit(&spi1_dev);    
//     int err = nrfx_spim_init(&spi1_dev, &spi1_config, NULL, NULL);
// 	if (err != NRFX_SUCCESS) {
// 		LOG_ERR("Failed to initialize SPI: 0x%08X\n", err);
// 		return;
// 	}
// }