/**
 * @file mpu6050.h
 * @brief MPU6050 driver header file
 * @author @MaxwellJay256
 * @version v2.0
 */
#ifndef _MPU6050_H_
#define _MPU6050_H_
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Edit these macros to fit your hardware
#define MPU6050_I2C_SDA 19             // MPU6050 I2C 的 SDA 引脚
#define MPU6050_I2C_SCL 18             // MPU6050 I2C 的 SCL 引脚
#define MPU6050_I2C_FREQ 400000        // MPU6050 I2C 的频率
#define MPU6050_I2C_PORT_NUM I2C_NUM_0 // MPU6050 的 I2C 端口号

/**
 * @brief MPU6050 配置参数
 * @param gyro_fsr 陀螺仪满量程范围, 0: ±250°/s; 1: ±500°/s; 2: ±1000°/s; 3: ±2000°/s
 * @param accel_fsr 加速度计满量程范围, 0: ±2g; 1: ±4g; 2: ±8g; 3: ±16g
 * @param sample_rate 采样率, 4 ~ 1000 Hz
 */
typedef struct {
    uint8_t gyro_fsr;
    uint8_t accel_fsr;
    uint16_t sample_rate;
} mpu6050_config_t;

/**
 * @brief 初始化 MPU6050
 * @return ESP_OK: Init success; Other: Fail
 */
esp_err_t mpu6050_init(mpu6050_config_t config);

/**
 * @brief 设置 MPU6050 的陀螺仪满量程范围
 * @param fsr 0: ±250°/s; 1: ±500°/s; 2: ±1000°/s; 3: ±2000°/s
 * @return ESP_OK: Execute success; Other: Fail
 */
esp_err_t mpu6050_set_gyro_fsr(uint8_t fsr);

/**
 * @brief 设置 MPU6050 的加速度计满量程范围
 * @param fsr 0: ±2g; 1: ±4g; 2: ±8g; 3: ±16g
 * @return ESP_OK: Execute success; Other: Fail
 */
esp_err_t mpu6050_set_accel_fsr(uint8_t fsr);

/**
 * @brief 设置 MPU6050 的低通滤波器频率
 * @param freq 数字低通滤波频率 (Hz)
 * @return ESP_OK: Execute success; Other: Fail
 */
esp_err_t mpu6050_set_LPF(uint16_t freq);

/**
 * @brief 设置 MPU6050 的采样率，设置的同时会设定 LPF 的频率
 * @param rate 4 ~ 1000 Hz. Value exceeding the range will be set to the nearest limit.
 * @return ESP_OK: Execute success; Other: Fail
 */
esp_err_t mpu6050_set_rate(uint16_t rate);

// Data structure definitions

/**
 * @brief MPU6050 的加速度数据
 */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_t;
/**
 * @brief MPU6050 的陀螺仪数据
 */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} gyro_t;
/**
 * @brief MPU6050 的完整输出数据，包含加速度计、温度计和陀螺仪
 */
typedef struct {
    accel_t accel;
    int16_t temp;
    gyro_t gyro;
} mpu6050_output_t;

/**
 * @brief 读取 MPU6050 的加速度计、陀螺仪和温度计数据
 * @return mpu6050_output_t 结构体，包含加速度计、陀螺仪和温度计数据
 */
mpu6050_output_t mpu6050_get_value();

/**
 * @brief 读取 MPU6050 的加速度计数据
 * @return accel_t {x, y, z} (原始数据，带符号)
 */
accel_t mpu6050_get_accel(void);
/**
 * @brief 读取 MPU6050 的陀螺仪数据
 * @return gyro_t {x, y, z} (原始数据，带符号)
 */
gyro_t mpu6050_get_gyro(void);
/**
 * @brief 读取 MPU6050 的温度计数据
 * @return 温度数值 (放大了 100 倍)
 */
uint8_t mpu6050_get_temperture(void);

// The following macros are from the MPU6050 datasheet, no edition allowed.
#define MPU6050_ADDR 0x68             // MPU6050 的 I2C 地址 b1101000(AD0)

#define MPU6050_SELF_TESTX_REG 0X0D   // 自检寄存器 X
#define MPU6050_SELF_TESTY_REG 0X0E   // 自检寄存器 Y
#define MPU6050_SELF_TESTZ_REG 0X0F   // 自检寄存器 Z
#define MPU6050_SELF_TESTA_REG 0X10   // 自检寄存器 A

#define MPU6050_SAMPLE_RATE_REG  0X19 // 采样频率分频器
#define MPU6050_CFG_REG          0X1A // 配置寄存器
#define MPU6050_GYRO_CFG_REG     0X1B // 陀螺仪配置寄存器
#define MPU6050_ACCEL_CFG_REG    0X1C // 加速度计配置寄存器
#define MPU6050_MOTION_DET_REG   0X1F // 运动检测阀值设置寄存器
#define MPU6050_FIFO_EN_REG      0X23 // FIFO 使能寄存器
#define MPU6050_I2CMST_CTRL_REG  0X24 // I2C 主机控制寄存器
#define MPU6050_I2CSLV0_ADDR_REG 0X25 // I2C 从机 0 器件地址寄存器
#define MPU6050_I2CSLV0_REG      0X26 // I2C 从机 0 数据地址寄存器
#define MPU6050_I2CSLV0_CTRL_REG 0X27 // I2C 从机 0 控制寄存器
#define MPU6050_I2CSLV1_ADDR_REG 0X28 // I2C 从机 1 器件地址寄存器
#define MPU6050_I2CSLV1_REG      0X29 // I2C 从机 1 数据地址寄存器
#define MPU6050_I2CSLV1_CTRL_REG 0X2A // I2C 从机 1 控制寄存器
#define MPU6050_I2CSLV2_ADDR_REG 0X2B // I2C 从机 2 器件地址寄存器
#define MPU6050_I2CSLV2_REG      0X2C // I2C 从机 2 数据地址寄存器
#define MPU6050_I2CSLV2_CTRL_REG 0X2D // I2C 从机 2 控制寄存器
#define MPU6050_I2CSLV3_ADDR_REG 0X2E // I2C 从机 3 器件地址寄存器
#define MPU6050_I2CSLV3_REG      0X2F // I2C 从机 3 数据地址寄存器
#define MPU6050_I2CSLV3_CTRL_REG 0X30 // I2C 从机 3 控制寄存器
#define MPU6050_I2CSLV4_ADDR_REG 0X31 // I2C 从机 4 器件地址寄存器
#define MPU6050_I2CSLV4_REG      0X32 // I2C 从机 4 数据地址寄存器
#define MPU6050_I2CSLV4_DO_REG   0X33 // I2C 从机 4 写数据寄存器
#define MPU6050_I2CSLV4_CTRL_REG 0X34 // I2C 从机 4 控制寄存器
#define MPU6050_I2CSLV4_DI_REG   0X35 // I2C 从机 4 读数据寄存器
#define MPU6050_I2CMST_STA_REG   0X36 // I2C 主机状态寄存器
#define MPU6050_INTBP_CFG_REG    0X37 // 中断/旁路设置寄存器
#define MPU6050_INT_EN_REG       0X38 // 中断使能寄存器
#define MPU6050_INT_STA_REG      0X3A // 中断状态寄存器

#define MPU6050_ACCEL_XOUTH_REG 0X3B // 加速度值, X 轴高 8 位寄存器
#define MPU6050_ACCEL_XOUTL_REG 0X3C // 加速度值, X 轴低 8 位寄存器
#define MPU6050_ACCEL_YOUTH_REG 0X3D // 加速度值, Y 轴高 8 位寄存器
#define MPU6050_ACCEL_YOUTL_REG 0X3E // 加速度值, Y 轴低 8 位寄存器
#define MPU6050_ACCEL_ZOUTH_REG 0X3F // 加速度值, Z 轴高 8 位寄存器
#define MPU6050_ACCEL_ZOUTL_REG 0X40 // 加速度值, Z 轴低 8 位寄存器

#define MPU6050_TEMP_OUTH_REG 0X41 // 温度值高 8 位寄存器
#define MPU6050_TEMP_OUTL_REG 0X42 // 温度值低 8 位寄存器

#define MPU6050_GYRO_XOUTH_REG 0X43 // 陀螺仪值, X 轴高 8 位寄存器
#define MPU6050_GYRO_XOUTL_REG 0X44 // 陀螺仪值, X 轴低 8 位寄存器
#define MPU6050_GYRO_YOUTH_REG 0X45 // 陀螺仪值, Y 轴高 8 位寄存器
#define MPU6050_GYRO_YOUTL_REG 0X46 // 陀螺仪值, Y 轴低 8 位寄存器
#define MPU6050_GYRO_ZOUTH_REG 0X47 // 陀螺仪值, Z 轴高 8 位寄存器
#define MPU6050_GYRO_ZOUTL_REG 0X48 // 陀螺仪值, Z 轴低 8 位寄存器
 
#define MPU6050_I2CSLV0_DO_REG 0X63 // I2C 从机 0 数据寄存器
#define MPU6050_I2CSLV1_DO_REG 0X64 // I2C 从机 1 数据寄存器
#define MPU6050_I2CSLV2_DO_REG 0X65 // I2C 从机 2 数据寄存器
#define MPU6050_I2CSLV3_DO_REG 0X66 // I2C 从机 3 数据寄存器

#define MPU6050_I2CMST_DELAY_REG 0X67 // I2C 主机延时管理寄存器
#define MPU6050_SIGPATH_RST_REG	 0X68 // 信号通道复位寄存器
#define MPU6050_MDETECT_CTRL_REG 0X69 // 运动检测控制寄存器
#define MPU6050_USER_CTRL_REG	 0X6A // 用户控制寄存器
#define MPU6050_PWR_MGMT1_REG    0X6B // 电源管理寄存器 1
#define MPU6050_PWR_MGMT2_REG	 0X6C // 电源管理寄存器 2
#define MPU6050_FIFO_CNTH_REG	 0X72 // FIFO 计数寄存器高 8 位
#define MPU6050_FIFO_CNTL_REG	 0X73 // FIFO 计数寄存器低 8 位
#define MPU6050_FIFO_RW_REG		 0X74 // FIFO 读写寄存器
#define MPU6050_DEVICE_ID_REG	 0X75 // 器件 ID 寄存器

#endif // _MPU6050_H_