#include "gps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

//GPS模块的串口号、引脚号和相关计算数据
#define UART_GPS UART_NUM_2
#define UART_GPS_TXD GPIO_NUM_16
#define UART_GPS_RXD GPIO_NUM_18
#define PI (3.1415926535898)
#define EARTH_RADIUS (6378.137)
#define detect_time 3 //!速度计算所用的时间间隔，目前

/// @brief GPS模块的串口数据缓存
char *GPS_temp_data = NULL;
static const int GPS_RX_BUF_SIZE = 1024;

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

/// @brief GPS数据结构体，目前仅使用经纬度
typedef struct {
    char lat[16];
    char lon[16];
    char utc[16];
    char day[8];
    char month[8];
    char year[8];
} gps_data_t;

/// @brief 角度转弧度
static double rad(double d)
{
    return d * PI / 180.0;
}

/// @brief 根据经纬度计算两点间距离
static double get_distance(double lat1, double lng1, double lat2, double lng2)
{
    double radLat1 = rad(lat1);
    double radLat2 = rad(lat2);
    double radLng1 = rad(lng1);
    double radLng2 = rad(lng2);
    double a = radLat1 - radLat2;
    double b = radLng1 - radLng2;
    double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2))); //计算球面两点间距离
    s *= EARTH_RADIUS;
    s *= 1000; //千米转为米
    return s;
}

/// @brief 根据两次距离计算平均速度
static double speed_caculate(double lat, double lon, double lat_, double lon_)
{
    double distance = get_distance(lat, lon, lat_, lon_);
    double speed = distance / detect_time;
    return speed;
}

/// @brief 截取字符串
static char *cut_substr(char *dest, char *src, char start, int n)
{
    char *p = dest;
    char *q = src;
    q += start;
    while(n--) *(p++ )= *(q++);
    *(p++) = '\0';
    return dest;
}

/// @brief 读取GPS模块的串口数据
static void read_uart_GPS(void *arg)
{
    static const char *GPS_TAG = "GPS";
    esp_log_level_set(GPS_TAG, ESP_LOG_INFO);
    gps_data_t gps_data;
    char *row = NULL;
    char *pos1 = NULL;
    char *pos2 = NULL;
    double lat_ = 0; //上一次的经纬度,隔一定的时间计算平均速度
    double lon_ = 0;
    double speed = 0;
    while (1)
    {
        bzero(&gps_data, sizeof(gps_data)); // 清空以便再次接收
        const int rxBytes = uart_read_bytes(UART_GPS, GPS_temp_data, GPS_RX_BUF_SIZE,1000 / portTICK_RATE_MS);
        ESP_LOGE(GPS_TAG, "rxBytes: %d\n", rxBytes);
        if (rxBytes > 0)
        {
            ESP_LOGI(GPS_TAG, "GPS_temp_data: %s\n", GPS_temp_data);//打印数据，测试用
            GPS_temp_data[rxBytes] = 0;
            row = strstr(GPS_temp_data, "$GNGGA");//截取GNGGA数据
            if(row == NULL)
            {
                ESP_LOGI(GPS_TAG, "row is NULL\n");
            }
            else
            {
                pos1 = strchr(row, ',');                        // UTC时间
                pos2 = strchr(pos1 + 1, ',');                   // 纬度
                pos1 = strchr(pos2 + 1, ',');                   // 纬度方向
                cut_substr(gps_data.lat, pos2, 1, pos1 - pos2); // 纬度
                pos2 = strchr(pos1 + 1, ',');                   // 经度
                pos1 = strchr(pos2 + 1, ',');                   // 经度方向
                cut_substr(gps_data.lon, pos2, 1, pos1 - pos2); // 经度
                double lat_transfor = atof(gps_data.lat);
                double lon_transfor = atof(gps_data.lon);
                // 转化为经纬度
                if (lat_transfor > 0)
                {
                    lat_transfor = (int)(lat_transfor / 100) + (lat_transfor - (int)(lat_transfor / 100) * 100) / 60;
                }
                if (lon_transfor > 0)
                {
                    lon_transfor = (int)(lon_transfor / 100) + (lon_transfor - (int)(lon_transfor / 100) * 100) / 60;
                }
                ESP_LOGI(GPS_TAG, "lat: %f,   lon: %f   \n", lat_transfor, lon_transfor);//输出经纬度，测试用
                speed = speed_caculate(lat_, lon_, lat_transfor, lon_transfor);
                ESP_LOGE("GPS", "speed: %f\n", speed);//输出速度，测试用
                lat_ = lat_transfor;
                lon_ = lon_transfor;
                ESP_LOGI(GPS_TAG, "======================================================\n");
            }
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    // free(GPS_temp_data);
}

/// @brief 初始化GPS模块
void init_GPS()
{
    // 8位数据位，无校验，1位停止位，无硬件流控制
	const uart_config_t uart_config_GPS = {
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_GPS, GPS_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_param_config(UART_GPS, &uart_config_GPS);
	uart_set_pin(UART_GPS, UART_GPS_TXD, UART_GPS_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI("GPS", "GPS init success!\n");
    // ESP_LOGI("RX_BUF_SIZE", "RX_BUF_SIZE: %d\n", GPS_RX_BUF_SIZE);
    GPS_temp_data = (char *)malloc(GPS_RX_BUF_SIZE + 1);
    xTaskCreate(read_uart_GPS, "read_uart_GPS", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);//!此处的任务优先级需要根据实际情况调整
}