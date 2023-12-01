/**
 * @file mpu6050.c
 * @brief MPU6050 driver source file
 * @author @MaxwellJay256
 * @version v1.0
*/
#include "mpu6050.h"

#define MPU6050_INIT_CMD_COUNT 11 // MPU6050 初始化指令的长度
/// @brief MPU6050 的初始化指令数组
static uint8_t mpu6050_init_cmd[MPU6050_INIT_CMD_COUNT][2] = {
    // 格式：{寄存器地址, 值}
    {0x6B, 0x80}, // PWR_MGMT_1，复位后 DEVICE_RESET 自动清零
    // 此处需要等待，见 mpu6050_init() 函数的 for 循环
    {0x6B, 0x00}, // 清除睡眠模式
    {0x1B, 0x18}, // 陀螺仪满量程配置 = ± 2000 °/s
    {0x1c, 0x00}, // 加速度计满量程配置 = ± 2g
    {0x38, 0x00}, // 禁用中断
    {0x6A, 0x00}, // 设置 AUX I2C 接口
    {0x23, 0x00}, // 设置 FIFO
    {0x19, 0x63}, // 设置采样率为 1 kHz / (1 + 99) = 10 Hz
    {0x1A, 0x13}, // 设置数字低通滤波器，带宽为 44 Hz
    {0x6B, 0x01}, // 设置系统时钟源为 X 轴陀螺仪 PLL 参考时钟
    {0x6C, 0x00}, // 使能传感器
};

#define ACK_CHECK_EN 0x1  // I2C 主机检查从机的 ACK
#define ACK_CHECK_DIS 0x0 // I2C 主机不检查从机的 ACK
#define ACK_VAL 0x0       // I2C 主机发送 ACK
#define NACK_VAL 0x1      // I2C 主机发送 NACK

/// @brief 从 MPU6050 的寄存器读取数据
/// @param i2c_num I2C 端口号
/// @param reg_addr 要读取的寄存器地址
/// @param data_rd 用于存储读取数据的缓冲区
/// @param size 要读取的数据长度
/// @return ESP_OK: Execute success
static esp_err_t mpu6050_i2c_read(
    i2c_port_t i2c_num, uint8_t *reg_addr, uint8_t *data_rd, size_t size)
{
    if (size == 0)
        return ESP_OK;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // 创建 I2C 指令
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    i2c_master_write_byte(cmd, *reg_addr, ACK_CHECK_EN); // 发送寄存器地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN); // 发送 MPU6050 地址和读取位
    if (size > 1)
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL); // 读取最后一个字节
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd); // 删除 I2C 指令
    return ret;
}

/// @brief 向 MPU6050 写入数据
/// @param i2c_num I2C 端口号
/// @param data_wr 要写入的数据
/// @param size 要写入的数据长度
/// @return ESP_OK: Execute success
static esp_err_t mpu6050_i2c_write(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // 创建 I2C 指令
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN); // 发送数据
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd); // 删除 I2C 指令
    return ret;
}

static const char *TAG = "MPU6050";
esp_err_t mpu6050_init()
{
    ESP_LOGI(TAG, "MPU6050 Initializing...");
    esp_err_t esp_err;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = MPU6050_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = MPU6050_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = MPU6050_I2C_FREQ,
    };

    esp_err = i2c_param_config(MPU6050_I2C_PORT_NUM, &conf);
    ESP_LOGI(TAG, "i2c_param_config: %d", esp_err);

    esp_err = i2c_driver_install(MPU6050_I2C_PORT_NUM, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "i2c_driver_install: %d", esp_err);

    for (size_t i = 0; i < MPU6050_INIT_CMD_COUNT; i++)
    {
        esp_err = mpu6050_i2c_write(MPU6050_I2C_PORT_NUM, mpu6050_init_cmd[i], 2);
        if (i == 0)
            vTaskDelay(500 / portTICK_RATE_MS); // 等待复位完成
    }
    ESP_LOGI(TAG, "mpu6050_init_cmd: %d", esp_err);
    return esp_err;
}

mpu6050_output_t mpu6050_get_value()
{
    uint8_t *measurement_bytes_out = (uint8_t *)malloc(14);
    uint8_t reg_addr = 0x3B;
    mpu6050_i2c_read(MPU6050_I2C_PORT_NUM, &reg_addr, measurement_bytes_out, 14);
    mpu6050_output_t measurement_out = {
        .accel = {
            .x = (int16_t)(measurement_bytes_out[0] << 8) | measurement_bytes_out[1],
            .y = (int16_t)(measurement_bytes_out[2] << 8) | measurement_bytes_out[3],
            .z = (int16_t)(measurement_bytes_out[4] << 8) | measurement_bytes_out[5],
        },
        .temp = (int16_t)(measurement_bytes_out[6] << 8) | measurement_bytes_out[7],
        .gyro = {
            .x = (int16_t)(measurement_bytes_out[8] << 8) | measurement_bytes_out[9],
            .y = (int16_t)(measurement_bytes_out[10] << 8) | measurement_bytes_out[11],
            .z = (int16_t)(measurement_bytes_out[12] << 8) | measurement_bytes_out[13],
        },
    };

    return measurement_out;
}
