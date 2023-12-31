/**
 * @file gps.h
 * @brief GPS driver header file
 */
#ifndef __GPS_H__
#define __GPS_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
#define UART_GPS UART_NUM_0
#define UART_GPS_TXD GPIO_NUM_1
#define UART_GPS_RXD GPIO_NUM_3

/// @brief GPS数据结构体
typedef struct GPS_data_
{
    double latitude;
    double longitude;
    double speed_kmh; // 单位：千米每小时
    double speed_ms;  // 单位：米每秒
    double course;  // 航向角，单位：度
    double altitude; // 海拔高度，单位：米
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
} GPS_data;

typedef struct UTC_time
{
    int hour;
    int minute;
    int second;
} UTCtime;

typedef struct UTC_date
{
    int day;
    int month;
    int year;
} UTCdate;

/// @brief 初始化 GPS模块
esp_err_t GPS_init();

/// @brief 从 GPS模块读取数据
GPS_data gps_get_value(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GPS_H__