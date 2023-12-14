/**
 * @file gps.c
 * @brief GPS driver source file
 */
#include "gps.h"

char *GPS_temp_data = NULL; // GPS模块输出数据缓冲区
static const int GPS_RX_BUF_SIZE = 1024;
static const char *TAG = "GPS";

/// @brief 读取GPS模块的串口数据
GPS_data get_gps_value()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    GPS_data gps_data;
    // bzero(&gps_data, sizeof(gps_data)); // 清空
    const int rxBytes = uart_read_bytes(UART_GPS, GPS_temp_data, GPS_RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
    ESP_LOGE(TAG, "rxBytes: %d\n", rxBytes);
    if (rxBytes > 0)
    {
        // ESP_LOGI(GPS_TAG, "GPS_temp_data: %s\n", GPS_temp_data);//打印数据，测试用
        GPS_temp_data[rxBytes] = 0;
        char *row = strstr(GPS_temp_data, "$GNRMC"); // 截取GNRMC数据
        if (row == NULL)
        {
            ESP_LOGI(TAG, "row is NULL\n");
        }
        else
        {
            char *token = strtok(row, ",");
            int count = 0;
            while (token != NULL)
            {
                count++;
                switch (count)
                {
                case 4: // 纬度
                    gps_data.latitude = atof(token) / 100;
                    break;
                case 6: // 经度
                    gps_data.longitude = atof(token) / 100;
                    break;
                case 8:                                           // 速度
                    gps_data.speed_kmh = atof(token);             // 默认为节
                        gps_data.speed_kmh *= 1.852;                  // 节转换为千米每小时
                    gps_data.speed_ms = gps_data.speed_kmh / 3.6; // 千米每小时转换为米每秒
                    break;
                }
                token = strtok(NULL, ",");
            }
            // ESP_LOGI(TAG, "lat: %f,   lon: %f   \n", gps_data.latitude, gps_data.longitude); // 输出经纬度，测试用
            //     ESP_LOGE(TAG, "speed: %f\n", gps_data.speed_ms); // 输出速度，测试用
            // ESP_LOGI(TAG, "======================================================\n");
        }
    }
    return gps_data;
}



/// @brief 初始化GPS模块
esp_err_t GPS_init()
{
    ESP_LOGI(TAG, "GPS ATGM336H Initializing...\n");
    esp_err_t esp_err;
    // 8位数据位，无校验，1位停止位，无硬件流控制，模块默认波特率9600（可修改）
    const uart_config_t uart_config_GPS = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    esp_err = uart_param_config(UART_GPS, &uart_config_GPS);
    ESP_LOGI(TAG, "uart_param_config: %d\n", esp_err);
    esp_err = uart_driver_install(UART_GPS, GPS_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "uart_driver_install: %d\n", esp_err);
    esp_err = uart_set_pin(UART_GPS, UART_GPS_TXD, UART_GPS_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "uart_set_pin: %d\n", esp_err);
    GPS_temp_data = (char *)malloc(GPS_RX_BUF_SIZE + 1);
    ESP_LOGI(TAG, "GPS init success!\n");
    return esp_err;
}