#pragma once
#include <zephyr.h>
#include "saadc.h"
#include "timer.h"
#include "ble_data_buf.h"
#include "ble.h"
#include <stdio.h>

typedef struct{
    bool sampling_enabled;
    data_buffer_t* p_buffer;
}ble_send_thread_app_t;

extern ble_send_thread_app_t ble_send_app;

void init_ble_send_app(void);

void ble_data_send_thread_func (void);