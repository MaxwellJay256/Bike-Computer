/**
 * @file gps.c
 * @brief GPS driver source file
*/
#include "gps.h"


/// @brief GPS模块的串口数据缓存
char *GPS_temp_data = NULL;
char *row = NULL;
static const int GPS_RX_BUF_SIZE = 1024;
    

static const char *GPS_TAG = "GPS";

//GPS模块输出数据例子
char *test_data = "\
$GNGGA,132506.000,2233.87430,N,11407.13740,E,1,13,1.0,103.3,M,-2.8,M,,*5E\n\
$GNGLL,2233.87430,N,11407.13740,E,132506.000,A,A*4E\n\
$GNGSA,A,3,02,05,15,23,24,29,195,,,,,,1.6,1.0,1.3,1*07\n\
$GNGSA,A,3,07,10,16,21,34,42,,,,,,,1.6,1.0,1.3,4*33\n\
$GPGSV,3,1,09,02,34,134,15,05,40,044,14,15,71,308,25,18,32,326,26,0*68\n\
$GPGSV,3,2,09,20,25,074,,23,13,293,37,24,32,174,31,29,45,251,37,0*6B\n\
$GPGSV,3,3,09,195,50,158,31,0*6A\n\
$BDGSV,3,1,11,03,,,28,07,13,193,27,10,14,207,32,12,,,35,0*71\n\
$BDGSV,3,2,11,16,66,191,29,21,47,308,41,22,41,027,,34,33,309,25,0*74\n\
$BDGSV,3,3,11,40,,,33,42,12,265,36,44,,,28,0*4B\n\
$GNRMC,132506.000,A,2233.87430,N,11407.13740,E,0.00,244.71,080522,,,A,V*0A\n\
$GNVTG,244.71,T,,M,0.00,N,0.00,K,A*27\n\
$GNZDA,132506.000,08,05,2022,00,00*44\n\
$GPTXT,01,01,01,ANTENNA OPEN*25\n\
";



typedef struct {
    double latitude;
    double longitude;
    double speed;
} GPSData;


/// @brief 读取GPS模块的串口数据
static void read_uart_GPS(void *arg)
{
    esp_log_level_set(GPS_TAG, ESP_LOG_INFO);
    GPSData gps_data;
    while (1)
    {
        bzero(&gps_data, sizeof(gps_data)); // 清空
        const int rxBytes = uart_read_bytes(UART_GPS, GPS_temp_data, GPS_RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        ESP_LOGE(GPS_TAG, "rxBytes: %d\n", rxBytes);
        if (rxBytes > 0)
        {
            // ESP_LOGI(GPS_TAG, "GPS_temp_data: %s\n", GPS_temp_data);//打印数据，测试用
            GPS_temp_data[rxBytes] = 0;
            row = strstr(GPS_temp_data, "$GNRMC"); // 截取GNRMC数据
            if (row == NULL)
            {
                ESP_LOGI(GPS_TAG, "row is NULL\n");
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
                    case 8: // 速度
                        gps_data.speed = atof(token);
                        gps_data.speed *= 1.852; // 节转换为千米每小时
                        break;
                    }
                    token = strtok(NULL, ",");
                }
                ESP_LOGI(GPS_TAG, "lat: %f,   lon: %f   \n", gps_data.latitude, gps_data.longitude); // 输出经纬度，测试用
                    ESP_LOGE(GPS_TAG, "speed: %f\n", gps_data.speed); // 输出速度，测试用
                ESP_LOGI(GPS_TAG, "======================================================\n");
                // 待完善：任务无法
            }
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    free(GPS_temp_data);
}

/// @brief 初始化GPS模块
esp_err_t GPS_init()
{
    ESP_LOGI(GPS_TAG, "GPS ATGM336H Initializing...\n");
    esp_err_t esp_err;
    // 8位数据位，无校验，1位停止位，无硬件流控制
	const uart_config_t uart_config_GPS = {
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    esp_err = uart_param_config(UART_GPS, &uart_config_GPS);
    ESP_LOGI(GPS_TAG, "uart_param_config: %d\n", esp_err);
    esp_err = uart_driver_install(UART_GPS, GPS_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
	ESP_LOGI(GPS_TAG, "uart_driver_install: %d\n", esp_err);
    esp_err = uart_set_pin(UART_GPS, UART_GPS_TXD, UART_GPS_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(GPS_TAG, "uart_set_pin: %d\n", esp_err);
    GPS_temp_data = (char *)malloc(GPS_RX_BUF_SIZE + 1);
    ESP_LOGI(GPS_TAG, "GPS init success!\n");
    xTaskCreate(read_uart_GPS, "read_uart_GPS", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);//!此处的任务优先级需要根据实际情况调整
    return esp_err;
}