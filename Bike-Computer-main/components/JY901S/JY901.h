// jy901.h
#ifndef _JY901_H_
#define _JY901_H_ 

#include <stdint.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 初始化UART
#define UART_NUM UART_NUM_0 //选择串口号
#define BUF_SIZE 256 //缓冲区大小
#define BAUD_RATE 9600 //波特率

// 定义JY901的数据结构体
typedef struct
{
  short roll; // 横滚角
  short pitch; // 俯仰角
  short yaw; // 偏航角
  short accX; // X轴加速度
  short accY; // Y轴加速度
  short accZ; // Z轴加速度
  short gyroX; // X轴角速度
  short gyroY; // Y轴角速度
  short gyroZ; // Z轴角速度
  short magX; // X轴磁场
  short magY; // Y轴磁场
  short magZ; // Z轴磁场
} JY901_DATA;

// 初始化 JY901 串口
void jy901_init(void);

// 读取 JY901 数据
void jy901_read(void);

// 打印 JY901 数据
void jy901_print(void);

#endif
