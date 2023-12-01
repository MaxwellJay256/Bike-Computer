/**
 * @file mpu6050.h
 * @brief MPU6050 driver header file
 * @author @MaxwellJay256
 * @version v1.0
*/
#ifndef _MPU6050_H_
#define _MPU6050_H_
#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 以下宏定义待修改
#define MPU6050_I2C_SDA 19             // MPU6050 I2C 的 SDA 引脚
#define MPU6050_I2C_SCL 18             // MPU6050 I2C 的 SCL 引脚
#define MPU6050_I2C_FREQ 400000        // MPU6050 I2C 的频率
#define MPU6050_I2C_PORT_NUM I2C_NUM_0 // MPU6050 的 I2C 端口号
#define MPU6050_ADDR 0x68              // MPU6050 I2C 的地址 b110100(AD0)

/// @brief 初始化 MPU6050
/// @return ESP_OK: Init success
esp_err_t mpu6050_init();

typedef struct {
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } accel;
    int16_t temp;
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } gyro;
} mpu6050_output_t;

/// @brief 读取 MPU6050 的加速度计、陀螺仪和温度计数据
/// @return mpu6050_output_t 结构体，包含加速度计、陀螺仪和温度计数据
mpu6050_output_t mpu6050_get_value();

#endif // _MPU6050_H_