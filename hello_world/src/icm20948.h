#ifndef ICM20948_H_
#define ICM20948_H_

//ICM-20948 3-axis gyroscope, 3-axis accelerometer, 3-axis compass datasheet avaliable at
//https://invensense.tdk.com/wp-content/uploads/2016/06/DS-000189-ICM-20948-v1.3.pdf

#include <stdint.h>
#include "i2c.h"

//user bank 0 register map
#define ICM20948_WHO_AM_I 0x00
#define ICM20948_USER_CTRL 0x03
#define ICM20948_LP_CONFIG 0x05
#define ICM20948_PWR_MGMT_1 0x06
#define ICM20948_PWR_MGMT_2 0x07
#define ICM20948_INT_PIN_CFG 0x0F
#define ICM20948_INT_ENABLE 0x10
#define ICM20948_INT_ENABLE_1 0x11
#define ICM20948_INT_ENABLE_2 0x12
#define ICM20948_INT_ENABLE_3 0x13
#define ICM20948_I2C_MST_STATUS 0x17
#define ICM20948_INT_STATUS 0x19
#define ICM20948_INT_STATUS_1 0x1A
#define ICM20948_INT_STATUS_2 0x1B
#define ICM20948_INT_STATUS_3 0x1C
#define ICM20948_DELAY_TIMEH 0x28
#define ICM20948_DELAY_TIMEL 0x29
#define ICM20948_ACCEL_XOUT_H 0x2D
#define ICM20948_ACCEL_XOUT_L 0x2E
#define ICM20948_ACCEL_YOUT_H 0x2F
#define ICM20948_ACCEL_YOUT_L 0x30
#define ICM20948_ACCEL_ZOUT_H 0x31
#define ICM20948_ACCEL_ZOUT_L 0x32
#define ICM20948_GYRO_XOUT_H 0x33
#define ICM20948_GYRO_XOUT_L 0x34
#define ICM20948_GYRO_YOUT_H 0x35
#define ICM20948_GYRO_YOUT_L 0x36
#define ICM20948_GYRO_ZOUT_H 0x37
#define ICM20948_GYRO_ZOUT_L 0x38
#define ICM20948_TEMP_OUT_H 0x39
#define ICM20948_TEMP_OUT_L 0x3A
#define ICM20948_EXT_SLV_SENS_DATA_00 0x3B
#define ICM20948_EXT_SLV_SENS_DATA_01 0x3C
#define ICM20948_EXT_SLV_SENS_DATA_02 0x3D
#define ICM20948_EXT_SLV_SENS_DATA_03 0x3E
#define ICM20948_EXT_SLV_SENS_DATA_04 0x3F
#define ICM20948_EXT_SLV_SENS_DATA_05 0x40
#define ICM20948_EXT_SLV_SENS_DATA_06 0x41
#define ICM20948_EXT_SLV_SENS_DATA_07 0x42
#define ICM20948_EXT_SLV_SENS_DATA_08 0x43
#define ICM20948_EXT_SLV_SENS_DATA_09 0x44
#define ICM20948_EXT_SLV_SENS_DATA_10 0x45
#define ICM20948_EXT_SLV_SENS_DATA_11 0x46
#define ICM20948_EXT_SLV_SENS_DATA_12 0x47
#define ICM20948_EXT_SLV_SENS_DATA_13 0x48
#define ICM20948_EXT_SLV_SENS_DATA_14 0x49
#define ICM20948_EXT_SLV_SENS_DATA_15 0x4A
#define ICM20948_EXT_SLV_SENS_DATA_16 0x4B
#define ICM20948_EXT_SLV_SENS_DATA_17 0x4C
#define ICM20948_EXT_SLV_SENS_DATA_18 0x4D
#define ICM20948_EXT_SLV_SENS_DATA_19 0x4E
#define ICM20948_EXT_SLV_SENS_DATA_20 0x4F
#define ICM20948_EXT_SLV_SENS_DATA_21 0x50
#define ICM20948_EXT_SLV_SENS_DATA_22 0x51
#define ICM20948_EXT_SLV_SENS_DATA_23 0x52
#define ICM20948_FIFO_EN_1 0x66
#define ICM20948_FIFO_EN_2 0x67
#define ICM20948_FIFO_RST 0x68
#define ICM20948_FIFO_MODE 0x69
#define ICM20948_FIFO_COUNTH 0x70
#define ICM20948_FIFO_COUNTL 0x71
#define ICM20948_FIFO_R_W 0x72
#define ICM20948_DATA_RDY_STATUS 0x74
#define ICM20948_FIFO_CFG 0x76
#define ICM20948_REG_BANK_SEL 0x7F

//user bank 1 register map
#define ICM20948_SELF_TEST_X_GYRO 0x02
#define ICM20948_SELF_TEST_Y_GYRO 0x03
#define ICM20948_SELF_TEST_Z_GYRO 0x04
#define ICM20948_SELF_TEST_X_ACCEL 0x0E
#define ICM20948_SELF_TEST_Y_ACCEL 0x0F
#define ICM20948_SELF_TEST_Z_ACCEL 0x10
#define ICM20948_XA_OFFS_H 0x14
#define ICM20948_XA_OFFS_L 0x15
#define ICM20948_YA_OFFS_H 0x17
#define ICM20948_YA_OFFS_L 0x18
#define ICM20948_ZA_OFFS_H 0x1A
#define ICM20948_ZA_OFFS_L 0x1B
#define ICM20948_TIMEBASE_CORRECTION_PLL 0x28
#define ICM20948_REG_BANK_SEL 0x7F

//user bnak 2 register map
#define ICM20948_GYRO_SMPLRT_DIV 0x00
#define ICM20948_GYRO_CONFIG_1 0x01
#define ICM20948_GYRO_CONFIG_2 0x02
#define ICM20948_XG_OFFS_USRH 0x03
#define ICM20948_XG_OFFS_USRL 0x04
#define ICM20948_YG_OFFS_USRH 0x05
#define ICM20948_YG_OFFS_USRL 0x06
#define ICM20948_ZG_OFFS_USRH 0x07
#define ICM20948_ZG_OFFS_USRL 0x08
#define ICM20948_ODR_ALIGN_EN 0x09
#define ICM20948_ACCEL_SMPLRT_DIV_1 0x10
#define ICM20948_ACCEL_SMPLRT_DIV_2 0x11
#define ICM20948_ACCEL_INTEL_CTRL 0x12
#define ICM20948_ACCEL_WOM_THR 0x13
#define ICM20948_ACCEL_CONFIG 0x14
#define ICM20948_ACCEL_CONFIG_2 0x15
#define ICM20948_FSYNC_CONFIG 0x52
#define ICM20948_TEMP_CONFIG 0x53
#define ICM20948_MOD_CTRL_USR 0x54
#define ICM20948_REG_BANK_SEL 0x7F

//user bank 3 register map
#define ICM20948_I2C_MST_ODR_CONFIG 0x00
#define ICM20948_I2C_MST_CTRL 0x01
#define ICM20948_I2C_MST_DELAY_CTRL 0x02
#define ICM20948_I2C_SLV0_ADDR 0x03
#define ICM20948_I2C_SLV0_REG 0x04
#define ICM20948_I2C_SLV0_CTRL 0x05
#define ICM20948_I2C_SLV0_DO 0x06
#define ICM20948_I2C_SLV1_ADDR 0x07
#define ICM20948_I2C_SLV1_REG 0x08
#define ICM20948_I2C_SLV1_CTRL 0x09
#define ICM20948_I2C_SLV1_DO 0x0A
#define ICM20948_I2C_SLV2_ADDR 0x0B
#define ICM20948_I2C_SLV2_REG 0x0C
#define ICM20948_I2C_SLV2_CTRL 0x0D
#define ICM20948_I2C_SLV2_DO 0x0E
#define ICM20948_I2C_SLV3_ADDR 0x0F
#define ICM20948_I2C_SLV3_REG 0x10
#define ICM20948_I2C_SLV3_CTRL 0x11
#define ICM20948_I2C_SLV3_DO 0x12
#define ICM20948_I2C_SLV4_ADDR 0x13
#define ICM20948_I2C_SLV4_REG 0x14
#define ICM20948_I2C_SLV4_CTRL 0x15
#define ICM20948_I2C_SLV4_DO 0x16
#define ICM20948_I2C_SLV4_DI 0x17
#define ICM20948_REG_BANK_SEL 0x7F

//The regiset map of the AK09916 (ICM-20948's magnetometer)
#define AK09916_WIA2 0x01
#define AK09916_ST1 0x10
#define AK09916_HXL 0x11
#define AK09916_HXH 0x12
#define AK09916_HYL 0x13
#define AK09916_HYH 0x14
#define AK09916_HZL 0x15
#define AK09916_HZH 0x16
#define AK09916_ST2 0x18
#define AK09916_CNTL2 0x31
#define AK09916_CNTL3 0x32
#define AK09916_TS1 0x33
#define AK09916_TS2 0x34

//@brief the icm20949 devices info
struct icm20948_info{
    //icm20948 have 3 register user bank
    uint8_t user_bank;

    uint8_t sleep_mode;

    //slave id of icm20948, id is 0b110110x the LSB bit depends on AD0 pin, according the schematic is 0x69 (0b1101001)
    //https://cdn.sparkfun.com/assets/c/7/8/f/8/15335_9DoF_Schematic.pdf
    const uint8_t slave_id;

    //enable fifo
    uint8_t fifo_en;
    //enable i2c master module
    uint8_t i2c_mst_en;

    //enable ODR alignment before set to gyro_smplrt_div, accel_smplrt_div, i2c_mst_odr_config
    uint8_t odr_align_en;   
    uint8_t gyro_smplrt_div;    //(8 bit), 1125/(1+gyro_smplrt_div)Hz
    uint16_t accel_smplrt_div;  //(12 bit), 1125/(1+accel_smplrt_div)Hz
    uint8_t i2c_mst_odr_config; //(4 bit), 1.1kHz/(2^odr_config), only work when gyro and accel is disable.

    uint8_t accel_fs;   //accelerometer sensitivity scale factor (2 bit)
    uint8_t gyro_fs;    //gyroscope snesitivity scale factor (2 bit)

    uint8_t accel_filt_en;      //enable the accelormeter on chip low pass filter
    uint8_t accel_filt_mode;

    uint8_t gyro_filt_en;      //enalbe the gyroscope on chip low pass filter
    uint8_t gyro_filt_mode;

    
};

extern struct icm20948_info icm20948_device0;

void init_icm20948(struct icm20948_info* icm20948);
void write_icm20948(struct icm20948_info* icm20948, uint8_t register_addr, uint8_t value);
uint8_t read_icm20948(struct icm20948_info* icm20948, uint8_t register_addr);
void read_icm20948_burst_read(struct icm20948_info* icm20948, uint8_t register_address, uint8_t *rx_data, uint8_t rx_data_len);
uint8_t write_icm20948_verify(struct icm20948_info* icm20948, uint8_t register_addr, uint8_t value);
void icm20948_set_user_bank(struct icm20948_info* icm20948, uint8_t data);
void icm20948_sleep(struct icm20948_info* icm20948, uint8_t enable);
void icm20948_odr_algi(struct icm20948_info* icm20948, uint8_t enable);
void icm20948_set_accel_smplrt_div(struct icm20948_info* icm20948, uint16_t divider);
void icm20948_set_accel_scale(struct icm20948_info* icm20948, uint8_t sacle);
void icm20948_accel_filter(struct icm20948_info* icm20948, uint8_t enable);
void icm20948_set_accel_filter(struct icm20948_info* icm20948, uint8_t mode);
void icm20948_set_gyro_smplrt_div(struct icm20948_info* icm20948, uint8_t divider);
void icm20948_set_gyro_scale(struct icm20948_info* icm20948, uint8_t scale);
void icm20948_gyro_filter(struct icm20948_info* icm20948, uint8_t enable);
void icm20948_set_gyro_filter(struct icm20948_info* icm20948, uint8_t mode);
uint16_t get_accel_x(struct icm20948_info* icm20948);
uint16_t get_accel_y(struct icm20948_info* icm20948);
uint16_t get_accel_z(struct icm20948_info* icm20948);
uint16_t get_gyro_x(struct icm20948_info* icm20948);
uint16_t get_gyro_y(struct icm20948_info* icm20948);
uint16_t get_gyro_z(struct icm20948_info* icm20948);
#endif