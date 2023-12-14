/**
 * @file gps.h
 * @brief GPS driver header file
*/

#ifndef __GPS_H__
#define __GPS_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//GPS模块的串口号、引脚号和相关计算数据
#define UART_GPS UART_NUM_2
#define UART_GPS_TXD GPIO_NUM_16
#define UART_GPS_RXD GPIO_NUM_18

/// @brief GPS数据结构体
typedef struct
{
    double latitude;
    double longitude;
    double speed_kmh; // 单位：千米每小时
    double speed_ms;  // 单位：米每秒
} GPS_data;

/// @brief 初始化 GPS模块
esp_err_t GPS_init();

/// @brief 从 GPS模块读取数据
GPS_data get_gps_value();

#endif