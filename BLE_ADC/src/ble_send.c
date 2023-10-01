#include "ble_send.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(ble_send, CONFIG_LOG_DEFAULT_LEVEL);

K_SEM_DEFINE(buffer_full_sem, 0, 16);

ble_send_thread_app_t ble_send_app;

static uint8_t test_counter = 0;

static void ble_send_buffer(data_buffer_t* buffer){
    uint8_t send_msg[STORAGE_BUFFER_SIZE] = {0};
    memcpy(send_msg, buffer->data, buffer->data_count);
    bt_nus_send(client, send_msg, buffer->data_count);
    clear_data_buffer(buffer);
}

static void add_saadc_data_to_ble_buf(void){
    if (!ble_send_app.sampling_enabled) return;
    data_buffer_t* buffer = write_data_buffer(ble_send_app.p_buffer, get_saadc_data());
    //data_buffer_t* buffer = write_data_buffer(ble_send_app.p_buffer, test_counter++);
    if (buffer != ble_send_app.p_buffer){
        ble_send_app.p_buffer = buffer;
        k_sem_give(&buffer_full_sem);
    }
}

void init_ble_send_app(void){
    ble_send_app.sampling_enabled = 0;
    ble_send_app.p_buffer = init_data_buffer();
    set_timer_app_cb(add_saadc_data_to_ble_buf);
}

void ble_data_send_thread_func (void){
    while(1){
        if (k_sem_take(&buffer_full_sem, K_FOREVER) != 0){
            continue;
        }
        data_buffer_t* full_buffer = get_full_buffer();
        ble_send_buffer(full_buffer);
    }
}