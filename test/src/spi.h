#pragma once

#include <stdint.h>
#include <stdio.h>
#include <nrfx_spim.h>

/* Configurations SPI, must match .overlay file for the chip. */
#define CONFIG_SPI_NAME               "SPI_4"  // Using SPI 4, the only high speed spi
#define CONFIG_SPI_SCK_PIN            8  // TODO: can we somehow retrieve these values from the .overlay file instead of having it hardcoded here ?
#define CONFIG_SPI_MOSI_PIN           9
#define CONFIG_SPI_MISO_PIN           10
#define CONFIG_SPI_CS_CTRL_GPIO_DEV   "GPIO_0"
#define CONFIG_SPI_CS_CTRL_GPIO_PIN   11  
#define CONFIG_SPI_FREQ_HZ            8000000
#define CONFIG_SPI_FREQ_NRF           NRF_SPIM_FREQ_16M

typedef struct spi_priv_t {
    bool is_initialized;
    nrfx_spim_t spi;
} spi_priv_t;

void spi_init(void);
void spi_test_send(void);
bool spi_is_initialized(void);
int spi_send_receive(uint8_t * send_buf, size_t send_length, uint8_t * recv_buf, size_t recv_length);

// void spi1_init(void);