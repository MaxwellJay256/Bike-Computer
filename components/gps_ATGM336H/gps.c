/**
 * @file gps.c
 * @brief GPS driver source file
 */
#include "gps.h"

char *GPS_temp_data = NULL; // GPS模块输出数据缓冲区
static const int GPS_RX_BUF_SIZE = 1024;
static const char *TAG = "GPS";

static UTCtime UTCTime_to_BJTime(double UTC_time)
{
    UTCtime BJ_time;
    BJ_time.hour = (int)(UTC_time / 10000 + 8) % 24;
    BJ_time.minute = (int)(UTC_time / 100) % 100;
    BJ_time.second = (int)UTC_time % 100;
    return BJ_time;
} 

static UTCdate UTCDate_to_BJDate(double UTC_date)
{
    UTCdate BJ_date;
    BJ_date.year = (int)(UTC_date / 10000);
    BJ_date.month = (int)(UTC_date / 100) % 100;
    BJ_date.day = (int)UTC_date % 100;
    return BJ_date;
}

/// @brief 读取GPS模块的串口数据
GPS_data get_gps_value()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    GPS_data gps_data = { 0 };
    const int rxBytes = uart_read_bytes(UART_GPS, GPS_temp_data, GPS_RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
    ESP_LOGE(TAG, "rxBytes: %d\n", rxBytes);
    if (rxBytes > 0)
    {
        ESP_LOGI(TAG, "GPS_temp_data: %s\n", GPS_temp_data); // 打印数据，测试用
        GPS_temp_data[rxBytes] = 0;
        char *row = strstr(GPS_temp_data, "$GNRMC");  // 截取GNRMC数据
        char *row2 = strstr(GPS_temp_data, "$GNGGA"); // 截取GPRMC数据
        if (row == NULL)
        {
            ESP_LOGI(TAG, "row is NULL\n");
            return gps_data;
        }
        else
        {
            char *token = strtok(row, ",");
            // ESP_LOGI(TAG, "tokenbefore: %s\n", token);
            int count = 0;
            UTCtime temp_BJtime;
            UTCdate temp_BJdate;
            while (count < 11)
            {
                count++;
                if (strcmp(token, "V") == 0)
                {
                    ESP_LOGI(TAG, "未定位\n");
                    return gps_data;
                }
                switch (count)
                {
                case 2: // UTC时间
                    temp_BJtime = UTCTime_to_BJTime(atof(token));
                    gps_data.hour = temp_BJtime.hour;
                    gps_data.minute = temp_BJtime.minute;
                    gps_data.second = temp_BJtime.second;
                    break;
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
                case 9: // 方位角
                    gps_data.course = atof(token);
                    break;
                case 10: // UTC日期
                    temp_BJdate = UTCDate_to_BJDate(atof(token));
                    gps_data.year = temp_BJdate.year;
                    gps_data.month = temp_BJdate.month;
                    gps_data.day = temp_BJdate.day;
                    break;
                }
                token = strtok(NULL, ",");
            }
            // ESP_LOGI(TAG, "lat: %f,   lon: %f   \n", gps_data.latitude, gps_data.longitude); // 输出经纬度，测试用
            //     ESP_LOGE(TAG, "speed: %f\n", gps_data.speed_ms); // 输出速度，测试用
            // ESP_LOGI(TAG, "======================================================\n");
            // 输出方位角和时间
            // ESP_LOGI(TAG, "course: %f\n", gps_data.course); // 输出方位角，测试用
            // ESP_LOGI(TAG, "time: %d:%d:%d\n", gps_data.hour, gps_data.minute, gps_data.second);
        }
        if (row2 == NULL)
        {
            ESP_LOGI(TAG, "row2 is NULL\n");
            gps_data.altitude = 0;
            return gps_data;
        }
        else
        {
            char *token = strtok(row2, ",");
            int count = 0;
            while (token != NULL)
            {
                count++;
                switch (count)
                {
                case 10: // 海拔
                    gps_data.altitude = atof(token);
                    break;
                }
                token = strtok(NULL, ",");
            }
            // ESP_LOGI(TAG, "alt: %f\n", gps_data.altitude); // 输出海拔，测试用
            // ESP_LOGI(TAG, "======================================================\n");
        }
    }
    else
    {
        ESP_LOGI(TAG, "rxBytes = 0\n");
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
