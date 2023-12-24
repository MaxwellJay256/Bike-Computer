# ATGM336H


## Origin output data form 


```c
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
```
GNRMC：Global Navigation Satellite System Recommended Minimum Specific GNSS Data   

GNGGA：Global Positioning System Fix Data  



典型GNRMC数据
``` 
$GNRMC,132506.000,A,2233.87430,N,11407.13740,E,0.00,244.71,080522,,,A,V*0A\n\
```

GNRMC解读  
字段 1：UTC时间，hhmmss.sss格式  
字段 2：状态，A=定位，V=未定位   
字段 3：纬度ddmm.mmmm，度分格式（前导位数不足则补0）  
字段 4：纬度N（北纬）或S（南纬）  
字段 5：经度dddmm.mmmm，度分格式（前导位数不足则补0）  
字段 6：经度E（东经）或W（西经）  
字段 7：速度，节，Knots（一节也是1.852千米／小时）  
字段 8：方位角，度（二维方向指向，相当于二维罗盘）  
字段 9：UTC日期，DDMMYY格式  
字段10：磁偏角，（000 - 180）度（前导位数不足则补0）  
字段11：磁偏角方向，E=东，W=西  
字段12：模式，A=自动，D=差分，E=估测，N=数据无效（3.0协议内容）  
字段13：校验值  
  

典型GNGGA数据$GNGGA,084024.000,2235.38880,N,11357.84205,E,1,18,0.8,13.0,M,0.0,M,,*4C
  

GNGGA解读  
字段 1：UTC 时间，格式为 hhmmss.ss；  
字段 2：纬度，格式为 ddmm.mmmmm（度分格式）；  
字段 3：纬度半球，N 或 S（北纬或南纬）；  
字段 4：经度，格式为 dddmm.mmmmm（度分格式）；
字段 5：经度半球，E 或 W（东经或西经）；
字段 6：GPS 状态，0=未定位，1=非差分定位，2=差分定位；  
字段 7：正在使用的用于定位的卫星数量（00~12）  
字段 8：HDOP 水平精确度因子（0.5~99.9）  
字段 9：海拔高度（-9999.9 到 9999.9 米）  
字段 10：大地水准面高度（-9999.9 到 9999.9 米）  
字段 11：差分时间（从最近一次接收到差分信号开始的秒数，非差分定位，此项为空）  
字段 12：差分参考基站标号（0000 到 1023，首位 0 也将传送，非差分定位，此项为空)


## Usage

### modify the macros 

```c
#define UART_GPS UART_NUM_2 //串口号
#define UART_GPS_TXD GPIO_NUM_16 //主控的TX引脚
#define UART_GPS_RXD GPIO_NUM_18 //主控的RX引脚
```

### use the API

#### esp_err_t GPS_init( )   
initialize related uart configuration

#### GPS_data get_gps_value( )
return GPS information with a GPS_data structure


```c
typedef struct
{
    double latitude;
    double longitude;
    double speed_kmh; // 单位：千米每小时
    double speed_ms;  // 单位：米每秒
} GPS_data;
```


## Example 

```c
#include gps.h  

void gps_task(void *arg)
{
    const char *TAG = "GPS";
    while (1)
    {
        GPS_data gps_data = get_gps_value();
        ESP_LOGI(TAG, "lat: %f,   lon: %f   \n", gps_data.latitude, gps_data.longitude); 
        ESP_LOGE(TAG, "speed: %f\n", gps_data,speed_ms); 
        ESP_LOGI(TAG, "======================================================\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}  

void app_main(void)
{
    GPS_init();
    xTaskCreate(&gps_task, "gps_task", 1024 * 2, NULL, 5, NULL);
}
```

or you can use other device like a SSD1306 OLED 

```c
#include gps.h 
#include fonts.h 
#include ssd1306.h  

void SSD1306_task(void *arg)
{
    const char *TAG = "SSD1306";
    while (1)
    {
        SSD1306_Clear();
        //输出GPS数据
        GPS_data gps_data = get_gps_value();
        char str[16];
        SSD1306_GotoXY(0, 0);
        sprintf(str, "lat: %f", gps_data.latitude);
        SSD1306_Puts(str, &Font_7x10, 1);
        SSD1306_GotoXY(0, 10);
        sprintf(str, "lon: %f", gps_data.longitude);
        SSD1306_Puts(str, &Font_7x10, 1);
        SSD1306_GotoXY(0, 20);
        sprintf(str, "speed: %f", gps_data.speed_ms);
        SSD1306_Puts(str, &Font_7x10, 1);
        SSD1306_UpdateScreen();
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
  
void app_main(void)
{
    GPS_init();
    SSD1306_Init();
    xTaskCreate(&SSD1306_task, "SSD1306_task", 1024 * 2, NULL, 5, NULL);
}
```
