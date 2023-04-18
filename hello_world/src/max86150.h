#ifndef MAX86150_H_
#define MAX86150_H_

//Datasheet of MAX86150 Integrated Photoplethysmogram and Electrocardiogram Bio-Sensor Module
//https://www.analog.com/media/en/technical-documentation/data-sheets/MAX86150.pdf

#include <stdint.h>
#include "i2c.h"

//slave id of max86150
#define MAX86150_ID 0b1011110
//Register map of MAX86150
//Status Register
#define MAX86150_INTERRUPT_STATUS_1     0x00
#define MAX86150_INTERRUPT_STATUS_2     0x01
#define MAX86150_INTERRUPT_ENABLE_1     0x02
#define MAX86150_INTERRUPT_ENABLE_2     0x03
//FIFO Register
#define MAX86150_FIFO_WRITE_POINTER     0x04
#define MAX86150_OVERFLOW_COUNTER     0x05
#define MAX86150_FIFO_READ_POINTER     0x06
#define MAX86150_FIFO_DATA_REGISTER     0x07
#define MAX86150_FIFO_CONFIGURATION     0x08
//FIFO Data Control
#define MAX86150_FIFO_DATA_CONTROL_REGISTER_1    0x09
#define MAX86150_FIFO_DATA_CONTROL_REGISTER_2     0x0A
//System Control
#define MAX86150_SYSTEM_CONTROL     0x0D
//PPG Configuration
#define MAX86150_PPG_CONFIGURATION_1     0x0E
#define MAX86150_PPG_CONFIGURATION_2     0x0F
#define MAX86150_PROX_INTERRUPT_THRESHOLD     0x10
//LED Pulse Amplitude
#define MAX86150_LED_1_PA     0x11
#define MAX86150_LED_2_PA     0x12
#define MAX86150_LED_RANGE     0x14
#define MAX86150_LED_PILOT_PA     0x15
//ECG Configuration
#define MAX86150_ECG_CONFIGURATION_1    0x3c
#define MAX86150_ECG_CONFIGURATION_2    0x3E
//Part ID
#define MAX86150_PART_ID     0xFF


/*@brief To write a max86150 register
*/
void write_max86150(uint8_t register_addr, uint8_t value){
    uint8_t data[2] = {register_addr, value};
    nrfx_err_t err;
    //s-->max86150 slave id(7 bit)-->w-->ack-->register address(8)-->ack-->data(8)-->p
    err = i2c1_send(MAX86150_ID, data, 2);
    if(err!= NRFX_SUCCESS) printk("max86150 i2c write register %d fail\n", register_addr);
}

/*@brief read a max86150 regsiter
*/
uint8_t read_max86150(uint8_t register_addr){
    uint8_t rx_data;
    uint8_t tx_data = register_addr;
    nrfx_err_t err;
    //s-->max86150 slave id(7 bit)-->w-->ack-->register address(8)-->ack-->sr-->max86150 slave id(7)-->read(1)-->ack-->data(8)-->nack-->p
    err = i2c1_send_follow_receive(MAX86150_ID,&tx_data,1,&rx_data,1);
    if(err!= NRFX_SUCCESS){
        printk("max86150 i2c register %d read fail.\n", register_addr);
        return 0;
    }
    return rx_data;
}

/*@brief write the max86150 then follow by a read command. This will verify the value is scuccessfully
 *write to max86150 by i2c bus with no data lost.
 *@return if the value is succesfully write to max86150 return 0 otherwise 1
*/
uint8_t write_max86150_verify(uint8_t register_addr, uint8_t value){
    write_max86150(register_addr,value);
    uint8_t read_value = read_max86150(register_addr);
    //this is a reset command, after reset the value of that register will become 0x00
    if (register_addr == MAX86150_SYSTEM_CONTROL && value == 0x01){
        if(read_value == 0x00) return 0;
    }
    if (read_value == value) return 0;
    else return 1;
}

/*@brief this function use the "how to configure the best performance" file as a reference to send sequence i2c
 *commands to max86150
*/
//https://www.analog.com/media/en/technical-documentation/design-notes/how-to-configure-the-optimal-settings-for-the-best-ppg-and-ecg-performance-in-the-max86150.pdf
void config_max86150(){
    uint8_t config_arr[][2] = {
        /*****Configure the FIFO Settings*****/
        //FIFO Almost full flag enable
        {MAX86150_INTERRUPT_ENABLE_1 ,0x80},
        //Reset, this bit will automatically become 0 after reset.
        {MAX86150_SYSTEM_CONTROL, 0x01},
        //Enable FIFO
        {MAX86150_SYSTEM_CONTROL, 0x04},
        //FIFO will roll over, and the FIFO almost full flag will rise when there is 15 (32 FIFO) empty space left
        {MAX86150_FIFO_CONFIGURATION, 0x1F},

        /*****Enabe the PPG and ECG Modes*****/
        //put PPG_LED2 data in FD2, and PPG_LED1 data in FD1
        {MAX86150_FIFO_DATA_CONTROL_REGISTER_1 ,0x21},
        //ECG data is put in FD3
        {MAX86150_FIFO_DATA_CONTROL_REGISTER_2 ,0x09},

        /*****Configure the Acquisition Setting for the Best PPG Performance*****/
        //if the LEDx_RGE = 00 the multiplier is .2 if 01 the multiplier is .4
        //The LED current = multiplier * the LEDx_PA value, in this case is 0x55*.2 = 17mA
        //LED1 and LED2 current setting
        {MAX86150_LED_1_PA  ,0x55}, 
        {MAX86150_LED_2_PA  ,0x55},
        //PPG_ADC_RGE = 0b11, full sacle ADC is 32768nA, with 62.5pA for each step, 2^19 steps 
        //PPG_SR = 0b0100, 100 samples per second, pulses per sample is 1
        //PPG_LED_PW = 0b11,the pulse width of LED is 400us
        {MAX86150_PPG_CONFIGURATION_1  ,0xD3},
        //
        {MAX86150_PPG_CONFIGURATION_2  ,0x18},

        /*****Configure the Acquisition Setting for the Best ECG Performance*****/
        //ECG sample rate 200Hz, bandwidth 70% filter 52Hz, bandwidth 90% 29Hz 
        {MAX86150_ECG_CONFIGURATION_1 ,0x03},
        //PGA_ECG_GAIN = 0b11, Gain of PGA ECG is 8
        //IA_GAIN =  0b01, Gain of instrucmental Amplifier is 9.5
        {MAX86150_ECG_CONFIGURATION_2 ,0x0D}

        /*****Configure the AFE Setting for the Best ECG performance*****/
    };
    uint8_t err = 0;
    for(int i=0;i<sizeof(config_arr)/sizeof(config_arr[0]);i++){
        do{
            err = write_max86150_verify(config_arr[i][0],config_arr[i][1]);
        }while(err);
    }
}

/*@brief this function will read multiple data from max86150 fifo
 *@param rx_data rx data buffer pointer
 *@param rx_data_len the number of data want to read from max86150, each read command will read 1 byte (8 bit)
*/
void read_max86150_fifo_burst_read(uint8_t *rx_data, uint8_t rx_data_len){
    uint8_t tx_data = MAX86150_FIFO_DATA_REGISTER;
    nrfx_err_t err;
    err = i2c1_send_follow_receive(MAX86150_ID,&tx_data,1,rx_data,rx_data_len);
    if(err!= NRFX_SUCCESS) printk("max86150 i2c fifo read fail.\n");
}

//there is 3 sensor on max86150 ppg LED1, ppg LED2, and ecg.
typedef struct{
    uint32_t ppg_led1_data;
    uint32_t ppg_led2_data;
    uint32_t ecg_data;
}max86150_data_t;

/*@brief this function read ppg and ecg data from max86150 fifo data register
 *@return max86150 data
*/
max86150_data_t max86150_get_data(){
    //Indefault config, only 3 FD is enabled. So only need 9 bytes
    uint8_t rx_data[9];
    read_max86150_fifo_burst_read(rx_data, sizeof(rx_data));
    //this is very specific to the way max86150 is config, the data depend which FDx it is put into
    //the sequence of rx_data is FD1 -> FD4, each has 3 bytes
    max86150_data_t data = {
        .ppg_led1_data = rx_data[0]<<16 | rx_data[1]<<8 | rx_data[2], 
        .ppg_led2_data = rx_data[3]<<16 | rx_data[4]<<8 | rx_data[5],
        .ecg_data = rx_data[6]<<16 | rx_data[7]<<8 | rx_data[8]
    };
    return data;
} 

#endif