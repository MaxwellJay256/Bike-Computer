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
#define PI (3.1415926535898)
#define EARTH_RADIUS (6378.137) //地球半径(千米)
#define detect_time 3 //!速度计算所用的时间间隔，目前


/// @brief 初始化 GPS模块
esp_err_t GPS_init();

#endif