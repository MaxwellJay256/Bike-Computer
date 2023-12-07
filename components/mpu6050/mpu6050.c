/**
 * @file mpu6050.c
 * @brief MPU6050 driver source file
 * @author @MaxwellJay256
 * @version v2.0
 */
#include "mpu6050.h"
static const char *TAG = "MPU6050";

static esp_err_t mpu6050_i2c_master_init();
static esp_err_t mpu6050_i2c_read(
    i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_rd, size_t size);
static esp_err_t mpu6050_i2c_write(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);
static esp_err_t mpu6050_i2c_write_byte(uint8_t reg_addr, uint8_t data_wr);
static esp_err_t mpu6050_i2c_read_byte(uint8_t reg_addr, uint8_t *data_rd);

esp_err_t mpu6050_init(mpu6050_config_t config)
{
    ESP_LOGI(TAG, "MPU6050 Initializing...");

    esp_err_t esp_err = mpu6050_i2c_master_init();

    // 设置 MPU6050
    mpu6050_i2c_write_byte(MPU6050_PWR_MGMT1_REG, 0x80); // 复位 MPU6050
    vTaskDelay(pdMS_TO_TICKS(100));                      // 等待复位完成
    mpu6050_i2c_write_byte(MPU6050_PWR_MGMT1_REG, 0x00); // 唤醒 MPU6050
    mpu6050_set_gyro_fsr(config.gyro_fsr);               // 设置陀螺仪满量程为 ± 2000 °/s
    mpu6050_set_accel_fsr(config.accel_fsr);             // 设置加速度计满量程为 ± 2g
    mpu6050_set_rate(config.sample_rate);                // 设置采样率为 50 Hz
    mpu6050_i2c_write_byte(MPU6050_INT_EN_REG, 0x00);    // 禁用中断
    mpu6050_i2c_write_byte(MPU6050_USER_CTRL_REG, 0x00); // 关闭 I2C 主模式
    mpu6050_i2c_write_byte(MPU6050_FIFO_EN_REG, 0x00);   // 关闭 FIFO
    mpu6050_i2c_write_byte(MPU6050_INTBP_CFG_REG, 0x80); // I2C 引脚低电平有效

    // 检查 MPU6050 是否初始化成功
    uint8_t mpu6050_device_id;
    esp_err = mpu6050_i2c_read_byte(MPU6050_DEVICE_ID_REG, &mpu6050_device_id);
    ESP_LOGI(TAG, "Get mpu6050_device_id: 0x%x", mpu6050_device_id);
    if (mpu6050_device_id == MPU6050_ADDR) {
        mpu6050_i2c_write_byte(MPU6050_PWR_MGMT1_REG, 0x01); // 设置系统时钟源为 X 轴陀螺仪 PLL 参考时钟
        mpu6050_i2c_write_byte(MPU6050_PWR_MGMT2_REG, 0x00); // 使能传感器
        mpu6050_set_rate(50); // 设置采样率为 50 Hz
        ESP_LOGI(TAG, "Init Success.");
    } else {
        ESP_LOGE(TAG, "Init Failed.");
        esp_err = ESP_FAIL;
    }

    return esp_err;
}

mpu6050_output_t mpu6050_get_value()
{
    uint8_t *measurement_bytes_out = (uint8_t *)malloc(14);
    // uint8_t reg_addr = 0x3B;
    mpu6050_i2c_read(MPU6050_I2C_PORT_NUM, MPU6050_ACCEL_XOUTH_REG, measurement_bytes_out, 14); // 从加速度计 X 轴高 8 位寄存器开始，读取 14 个字节
    mpu6050_output_t measurement_out = {
        .accel = {
            .x = (int16_t)(measurement_bytes_out[0] << 8) | measurement_bytes_out[1], // 将高 8 位和低 8 位合并，方法是高 8 位左移 8 位，然后和低 8 位做或运算
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

accel_t mpu6050_get_accel()
{
    accel_t accel_out;
    uint8_t buffer[6];
    esp_err_t esp_err = mpu6050_i2c_read(
        MPU6050_I2C_PORT_NUM, MPU6050_ACCEL_XOUTH_REG, buffer, 6);
    if (esp_err == ESP_OK) {
        accel_out.x = (uint16_t)(buffer[0] << 8) | buffer[1];
        accel_out.y = (uint16_t)(buffer[2] << 8) | buffer[3];
        accel_out.z = (uint16_t)(buffer[4] << 8) | buffer[5];
    } else {
        ESP_LOGE(TAG, "Get accelerometer data failed.");
        accel_out = (accel_t){0, 0, 0};
    }
    return accel_out;
}

gyro_t mpu6050_get_gyro(void)
{
    gyro_t gyro_out;
    uint8_t buffer[6];
    esp_err_t esp_err = mpu6050_i2c_read(
        MPU6050_I2C_PORT_NUM, MPU6050_GYRO_XOUTH_REG, buffer, 6);
    if (esp_err == ESP_OK) {
        gyro_out.x = (uint16_t)(buffer[0] << 8) | buffer[1];
        gyro_out.y = (uint16_t)(buffer[2] << 8) | buffer[3];
        gyro_out.z = (uint16_t)(buffer[4] << 8) | buffer[5];
    } else {
        ESP_LOGE(TAG, "Get gyroscope data failed.");
        gyro_out = (gyro_t){0, 0, 0};
    }
    return gyro_out;
}

uint8_t mpu6050_get_temperture(void)
{
    uint8_t buffer[2];
    short raw;
    float temp;

    esp_err_t esp_err = mpu6050_i2c_read(
        MPU6050_I2C_PORT_NUM, MPU6050_TEMP_OUTH_REG, buffer, 2);
    if (esp_err == ESP_OK) {
        raw = (buffer[0] << 8) | buffer[1];
        temp = (raw / 340.0) + 36.53;
    } else {
        ESP_LOGE(TAG, "Get temperature data failed.");
        temp = 0;
    }
    return temp * 100;
}

esp_err_t mpu6050_set_gyro_fsr(uint8_t fsr)
{
    ESP_LOGI(TAG, "Set gyroscopes full-scale range to ±%d °/s.", 250 << fsr);
    return mpu6050_i2c_write_byte(MPU6050_GYRO_CFG_REG, fsr << 3);
}

esp_err_t mpu6050_set_accel_fsr(uint8_t fsr)
{
    ESP_LOGI(TAG, "Set accelerometers full-scale range to ±%dg.", 2 << fsr);
    return mpu6050_i2c_write_byte(MPU6050_ACCEL_CFG_REG, fsr << 3);
}

esp_err_t mpu6050_set_LPF(uint16_t freq)
{
    uint8_t lpf = 0;
    if (freq >= 188) lpf = 1;
    else if (freq >= 98) lpf = 2;
    else if (freq >= 42) lpf = 3;
    else if (freq >= 20) lpf = 4;
    else if (freq >= 10) lpf = 5;
    else lpf = 6;

    ESP_LOGI(TAG, "Set low pass filter to %d Hz.", freq);
    return mpu6050_i2c_write_byte(MPU6050_CFG_REG, lpf);
}

esp_err_t mpu6050_set_rate(uint16_t rate)
{
    uint8_t data;
    if (rate > 1000) rate = 1000;
    if (rate < 4) rate = 4;
    data = 1000 / rate - 1;

    ESP_LOGI(TAG, "Set sampling rate to %d Hz.", rate);
    mpu6050_i2c_write_byte(MPU6050_SAMPLE_RATE_REG, data);
    return mpu6050_set_LPF(rate / 2);
}

/// @brief 初始化 MPU6050 的 I2C 总线
/// @return ESP_OK: Init success
static esp_err_t mpu6050_i2c_master_init()
{
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
    if (esp_err == ESP_OK) {
        ESP_LOGI(TAG, "i2c_param_config: ESP_OK");
    } else {
        ESP_LOGI(TAG, "i2c_param_config: %d", esp_err);
    }

    esp_err = i2c_driver_install(MPU6050_I2C_PORT_NUM, conf.mode, 0, 0, 0);
    if (esp_err == ESP_OK) {
        ESP_LOGI(TAG, "i2c_driver_install: ESP_OK");
    } else {
        ESP_LOGI(TAG, "i2c_driver_install: %d", esp_err);
    }
    return esp_err;
}

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
    i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
    if (size == 0)
        return ESP_OK;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN); // 发送寄存器地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN); // 发送 MPU6050 地址和读取位
    if (size > 1)
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000)); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd);
    return ret;
}

/// @brief 向 MPU6050 写入数据
/// @param i2c_num I2C 端口号
/// @param data_wr 要写入的数据
/// @param size 要写入的数据长度
/// @return ESP_OK: Write success
static esp_err_t mpu6050_i2c_write(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // 创建 I2C 指令
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN); // 发送数据
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000)); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd); // 删除 I2C 指令
    return ret;
}

/// @brief 向 MPU6050 的寄存器写入 1 个字节
/// @param reg_addr 寄存器地址
/// @param data_wr 要写入的数据
/// @return ESP_OK: Write success
static esp_err_t mpu6050_i2c_write_byte(uint8_t reg_addr, uint8_t data_wr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    esp_err_t ack = i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    if (ack == ESP_ERR_NO_MEM)
    {
        i2c_master_stop(cmd);
        return ESP_FAIL;
    }

    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN); // 发送寄存器地址
    esp_err_t ack1 = i2c_master_write_byte(cmd, data_wr, ACK_CHECK_EN); // 发送数据
    if (ack1 == ESP_ERR_NO_MEM)
    {
        i2c_master_stop(cmd);
        return ESP_FAIL;
    }
    i2c_master_stop(cmd);

    i2c_master_cmd_begin(MPU6050_I2C_PORT_NUM, cmd, pdMS_TO_TICKS(1000)); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

/// @brief 从 MPU6050 的寄存器读取 1 个字节
/// @param reg_addr 寄存器地址
/// @param data_rd 用于存储读取数据的 1 字节缓冲区
/// @return ESP_OK: Read success
static esp_err_t mpu6050_i2c_read_byte(uint8_t reg_addr, uint8_t *data_rd)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    esp_err_t ack = i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN); // 发送 MPU6050 地址和写入位
    if (ack == ESP_ERR_NO_MEM)
    {
        i2c_master_stop(cmd);
        return ESP_FAIL;
    }
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN); // 发送寄存器地址

    i2c_master_start(cmd);
    esp_err_t ack1 = i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN); // 发送 MPU6050 地址和读取位
    if (ack1 == ESP_ERR_NO_MEM)
    {
        i2c_master_stop(cmd);
        return ESP_FAIL;
    }

    i2c_master_read_byte(cmd, data_rd, NACK_VAL); // 读取数据
    i2c_master_stop(cmd);

    i2c_master_cmd_begin(MPU6050_I2C_PORT_NUM, cmd, pdMS_TO_TICKS(1000)); // 发送 I2C 指令
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}
