#include <zephyr.h>
#include <zephyr/kernel.h>

K_MSGQ_DEFINE(my_msgq, sizeof(uint8_t), 10, 4);

void data_producer(){
    uint8_t data;
    k_msgq_put(&my_msgq, &data, K_NO_WAIT);
}

void main(void){

}