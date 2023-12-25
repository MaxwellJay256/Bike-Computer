/**
 * @file jy901s.h
 * @author @chickencheng
 */
#ifndef _JY901S_H_
#define _JY901S_H_

#include <stdint.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 初始化UART
#define UART_NUM UART_NUM_0 // 选择串口号
#define BUF_SIZE 256        // 缓冲区大小
#define BAUD_RATE 9600      // 波特率
#define g 9.7887            // 深圳重力加速度
// 定义 JY901S 的数据结构体
typedef struct
{
  double roll;  // 横滚角
  double pitch; // 俯仰角
  double yaw;   // 偏航角
  double accX;  // X轴加速度
  double accY;  // Y轴加速度
  double accZ;  // Z轴加速度
  double gyroX; // X轴角速度
  double gyroY; // Y轴角速度
  double gyroZ; // Z轴角速度
  double magX;  // X轴磁场
  double magY;  // Y轴磁场
  double magZ;  // Z轴磁场
} JY901S_DATA;

// 初始化 JY901 串口
void jy901s_init(void);

// 读取并处理 JY901 数据
JY901S_DATA jy901s_read(JY901S_DATA jy901s_data);

// 打印 JY901 数据
void jy901s_printall(JY901S_DATA jy901s_data);

// 打印JY901的角度数据到串口
void jy901s_printangle(JY901S_DATA jy901s_data);

// 打印JY901的角速度数据到串口
void jy901s_printgyro(JY901S_DATA jy901s_data);

// 打印JY901的加速度数据到串口
void jy901s_printacc(JY901S_DATA jy901s_data);

// 打印JY901的磁场数据到串口
void jy901s_printmag(JY901S_DATA jy901s_data);

#endif // _JY901S_H_