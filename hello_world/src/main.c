#include <zephyr.h>
#include <sys/printk.h>
#include "max86150.h"
#include "icm20948.h"

void main(void)
{
	k_sleep(K_SECONDS(1));
	init_i2c1();
	k_sleep(K_SECONDS(1));
	//uint8_t error;
	//error = write_icm20948_verify(&icm20948_device0,ICM20948_PWR_MGMT_1,1);
	init_icm20948(&icm20948_device0);
	uint16_t data = 0;
	while(1){
		data = get_accel_x(&icm20948_device0);
		printk("accel x: %d,", data);
		data = get_gyro_x(&icm20948_device0);
		printk("gyro x: %d\n", data);
		k_sleep(K_MSEC(500));
	}
} 