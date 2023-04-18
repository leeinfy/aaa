#include "icm20948.h"

//@brief create an default instance of icm20948
struct icm20948_info icm20948_device0 = {.user_bank = 0,
                                         .sleep_mode = 1,
                                         .slave_id = 0b1101001,
                                         .fifo_en = 0,
                                         .i2c_mst_en = 0,
                                         .odr_align_en = 0,
                                         .accel_filt_en = 0,
};

//@brief init the icm20938 devices
void init_icm20948(struct icm20948_info* icm20948){
    icm20948_set_accel_filter(icm20948,7);
    icm20948_set_gyro_filter(icm20948,7);
    icm20948_sleep(icm20948, 0);
}

/*@brief write a icm20948 register
*/
void write_icm20948(struct icm20948_info* icm20948, uint8_t register_addr, uint8_t value){
    uint8_t data[2] = {register_addr, value};
    nrfx_err_t err;
    //s-->icm20948 slave id(7 bit)+w-->ack-->register address(8)-->ack-->data(8)-->p
    err = i2c1_send(icm20948->slave_id, data, 2);
    if(err!= NRFX_SUCCESS) printk("icm20948 i2c write register %d fail\n", register_addr);
}

/*@brief read a icm20948 regsiter
*/
uint8_t read_icm20948(struct icm20948_info* icm20948, uint8_t register_addr){
    uint8_t rx_data;
    uint8_t tx_data = register_addr;
    nrfx_err_t err;
    //s-->icm20948 slave id(7 bit)+w-->ack-->register address(8)-->ack-->sr-->icm20948 slave id(7)+r-->ack-->data(8)-->nack-->p
    err = i2c1_send_follow_receive(icm20948->slave_id,&tx_data,1,&rx_data,1);
    if(err!= NRFX_SUCCESS){
        printk("icm20948 i2c register %d read fail.\n", register_addr);
        return 0;
    }
    return rx_data;
}

/*@brief this function will read multiple register value from icm20948, after read one data (1 Byte) the register counter will increment by 1.
 *So, the next data is form (currnet register address + 1) 
 *@param rx_data rx data storage pointer
 *@param rx_data_len the number of data want to read from icm20948
*/
void read_icm20948_burst_read(struct icm20948_info* icm20948, uint8_t register_address, uint8_t *rx_data, uint8_t rx_data_len){
    uint8_t tx_data = register_address;
    nrfx_err_t err;
    err = i2c1_send_follow_receive(icm20948->slave_id,&tx_data,1,rx_data,rx_data_len);
    if(err!= NRFX_SUCCESS) printk("icm20948 i2c fifo read fail.\n");
}

/*@brief write the icm20948 then follow by a read command. This will verify the value is scuccessfully
 *write to icm20948 by i2c bus with no data lost.
 *@return if the value is succesfully write to icm20948 return 0 otherwise 1
*/
uint8_t write_icm20948_verify(struct icm20948_info* icm20948, uint8_t register_addr, uint8_t value){
    write_icm20948(icm20948,register_addr,value);
    uint8_t read_value = read_icm20948(icm20948, register_addr);
    if (read_value == value) return 0;
    else {
        printk("icm20948 i2c sent data lost\n");
        return 1;
    }
}

/*@brief set the user bank register 
 */
void icm20948_set_user_bank(struct icm20948_info* icm20948, uint8_t data){
    uint8_t err = 0;
    if (icm20948->user_bank != data){
        do{
            err = write_icm20948_verify(icm20948,ICM20948_REG_BANK_SEL, (data << 4));
        }while(err);
    }
    icm20948->user_bank = data;
}

/*@brief enable or disable the sleep mode
*/
void icm20948_sleep(struct icm20948_info* icm20948, uint8_t enable){
    icm20948_set_user_bank(icm20948,0);
    uint8_t data;
    uint8_t error;
    if(enable){
        if(!icm20948->sleep_mode){
            data =  read_icm20948(icm20948,ICM20948_PWR_MGMT_1);
            data |= (1<<6);
            error = write_icm20948_verify(icm20948,ICM20948_PWR_MGMT_1,data);
            icm20948->sleep_mode = 1;
        }
    }else if(!enable){
        if(icm20948->sleep_mode){
            data =  read_icm20948(icm20948,ICM20948_PWR_MGMT_1);
            data &= ~(1<<6);
            error = write_icm20948_verify(icm20948,ICM20948_PWR_MGMT_1,data);
            icm20948->sleep_mode = 0;
        }
    }
}

/*@brief enable or disable output sample rate alignment
*/
void icm20948_odr_algi(struct icm20948_info* icm20948, uint8_t enable){
    icm20948_set_user_bank(icm20948,2);
    uint8_t error;
    if(enable){
        if(!icm20948->odr_align_en){
            error = write_icm20948_verify(icm20948,ICM20948_PWR_MGMT_1, 0b1);
            icm20948->odr_align_en = 1;
        }
    }else if(!enable){
        if(icm20948->odr_align_en){
            error = write_icm20948_verify(icm20948,ICM20948_PWR_MGMT_1, 0b0);
            icm20948->odr_align_en = 0;
        }
    }
}

/*@brief set the accelormeter sample rate divider 
 *@param divider 12 bits
*/
void icm20948_set_accel_smplrt_div(struct icm20948_info* icm20948, uint16_t divider){
    icm20948_set_user_bank(icm20948,2);
    uint8_t err;
    uint8_t divider_h = divider >> 8;
    err = write_icm20948_verify(icm20948, ICM20948_ACCEL_SMPLRT_DIV_1, divider_h);
    uint8_t divider_l = divider & 0xFF;
    err = write_icm20948_verify(icm20948, ICM20948_ACCEL_SMPLRT_DIV_2, divider_l);
    icm20948->accel_smplrt_div = divider;
}

/*@brief set the accelormeter full scale
 *@param scale 00 +/-2g, 01 +/-4g, 10 +/-8g, 01 +/-16g
*/
void icm20948_set_accel_scale(struct icm20948_info* icm20948, uint8_t scale){
    icm20948_set_user_bank(icm20948,2);
    uint8_t data = read_icm20948(icm20948,ICM20948_ACCEL_CONFIG);
    data |= scale << 1;
    uint8_t err;
    err = write_icm20948_verify(icm20948,ICM20948_ACCEL_CONFIG, data);
    icm20948->accel_fs = scale;
}

/*@brief enable or disable icm20948 accelerometer low pass filter filter
*/
void icm20948_accel_filter(struct icm20948_info* icm20948, uint8_t enable){
    icm20948_set_user_bank(icm20948,2);
    uint8_t data;
    uint8_t error;
    if(enable){
        if(!icm20948->accel_filt_en){
            data =  read_icm20948(icm20948,ICM20948_ACCEL_CONFIG);
            data |= 0b1;
            error = write_icm20948_verify(icm20948,ICM20948_ACCEL_CONFIG,data);
            icm20948->accel_filt_en = 1;
        }
    }else if(!enable){
        if(icm20948->accel_filt_en){
            data =  read_icm20948(icm20948,ICM20948_ACCEL_CONFIG);
            data &= ~0b1;
            error = write_icm20948_verify(icm20948,ICM20948_ACCEL_CONFIG,data);
            icm20948->accel_filt_en = 0;
        }
    }
}

/*@brief set up the accelormeter low pass filter
 *@param mode check out table 18
*/
void icm20948_set_accel_filter(struct icm20948_info* icm20948, uint8_t mode){
    icm20948_set_user_bank(icm20948,2);
    uint8_t data = read_icm20948(icm20948,ICM20948_ACCEL_CONFIG);
    data |= (mode << 3) | 0b1;
    uint8_t err;
    err = write_icm20948_verify(icm20948,ICM20948_ACCEL_CONFIG, data);
    icm20948->accel_filt_en = 1;
    icm20948->accel_filt_mode = mode;
}

/*@brief set up the gyroscope sample rate divider
*/
void icm20948_set_gyro_smplrt_div(struct icm20948_info* icm20948, uint8_t divider){
    icm20948_set_user_bank(icm20948,2);
    uint8_t err;
    err = write_icm20948_verify(icm20948, ICM20948_GYRO_SMPLRT_DIV, divider);
    icm20948->gyro_smplrt_div = divider;
}

/*@brief set gyroscope the full scale
 *@param scale 00 +/-250 dps, 01 +/-500 dps, 10 +/-1000 dps, 11 +/-2000 dps,
*/
void icm20948_set_gyro_scale(struct icm20948_info* icm20948, uint8_t scale){
    icm20948_set_user_bank(icm20948,2);
    uint8_t err;
    uint8_t data = read_icm20948(icm20948,ICM20948_GYRO_CONFIG_1);
    data |= scale << 1;
    err = write_icm20948_verify(icm20948,ICM20948_GYRO_CONFIG_1, data);
    icm20948->gyro_fs = scale;
}

/*@brief enalbe or disable gyroscope filter
*/
void icm20948_gyro_filter(struct icm20948_info* icm20948, uint8_t enable){
    icm20948_set_user_bank(icm20948,2);
    uint8_t data;
    uint8_t error;
    if(enable){
        if(!icm20948->gyro_filt_en){
            data =  read_icm20948(icm20948,ICM20948_GYRO_CONFIG_1);
            data |= 0b1;
            error = write_icm20948_verify(icm20948,ICM20948_GYRO_CONFIG_1,data);
            icm20948->gyro_filt_en = 1;
        }
    }else if(!enable){
        if(icm20948->gyro_filt_en){
            data =  read_icm20948(icm20948,ICM20948_GYRO_CONFIG_1);
            data &= ~0b1;
            error = write_icm20948_verify(icm20948,ICM20948_GYRO_CONFIG_1,data);
            icm20948->gyro_filt_en = 0;
        }
    }    
}

/*@brief set up the accelormeter low pass filter
 *@param mode check out table 16
*/
void icm20948_set_gyro_filter(struct icm20948_info* icm20948, uint8_t mode){
    icm20948_set_user_bank(icm20948,2);
    uint8_t data = read_icm20948(icm20948,ICM20948_GYRO_CONFIG_1);
    data |= (mode << 3) | 0b1;
    uint8_t err;
    err = write_icm20948_verify(icm20948,ICM20948_GYRO_CONFIG_1, data);
    icm20948->gyro_filt_en = 1;
    icm20948->gyro_filt_mode = mode;
}

/*@brief get icm20948 accelerometer X-axis data (2 Bytes)
*/
uint16_t get_accel_x(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_ACCEL_XOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}

/*@brief get icm20948 accelerometer Y-axis data (2 Bytes)
*/
uint16_t get_accel_y(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    icm20948->user_bank = 0;
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_ACCEL_YOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}


/*@brief get icm20948 accelerometer Z-axis data (2 Bytes)
*/
uint16_t get_accel_z(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_ACCEL_ZOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}

/*@brief get icm20948 gyroscope X-axis data (2 Bytes)
*/
uint16_t get_gyro_x(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_GYRO_XOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}

/*@brief get icm20948 gyroscope Y-axis data (2 Bytes)
*/
uint16_t get_gyro_y(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_GYRO_YOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}

/*@brief get icm20948 gyroscope z-axis data (2 Bytes)
*/
uint16_t get_gyro_z(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_GYRO_ZOUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}

/*@brief get icm20948 temperature data (2 Bytes)
*/
uint16_t get_temp(struct icm20948_info* icm20948){
    icm20948_set_user_bank(icm20948,0);
    uint8_t rx_data[2];
    read_icm20948_burst_read(icm20948,ICM20948_TEMP_OUT_H,rx_data,2);
    return rx_data[0] << 8 | rx_data[1];
}