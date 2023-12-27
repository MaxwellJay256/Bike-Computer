/**
 * @file app_main.c
 * @brief Main application source code
*/
#include "app_main.h"
#include "bluetooth.h"
#include "images.h"
#include "jy901s.h"
#include "epd.h"
#include "gps.h"

#define IMU_UART_NUM 2
#define IMU_TX_PIN 17
#define IMU_RX_PIN 16
#define IMU_TASK_STACK_SIZE 2048

#define GPS_UART_NUM 0
#define GPS_TX_PIN 1
#define GPS_RX_PIN 3
#define GPS_TASK_STACK_SIZE 2048

/// @brief Main application entry point
extern "C" void app_main(void)
{
    jy901s_init(IMU_UART_NUM, IMU_TX_PIN, IMU_RX_PIN);
    GPS_init();
    epd_init_all();
    epd_clear_screen(EPD_WHITE);
    Paint ui;
    WINDOW window_full = {
        .x_start = 0,
        .y_start = 0,
        .width = 200,
        .height = 200,
    };
    JY901S_DATA imu_data;
    GPS_data gps_data;
    ui.clear();
    ui.set_image(gImage_ui);

    while (1)
    {
        imu_data = jy901s_read(IMU_UART_NUM);
        gps_data = gps_get_value();

        char latitude_str[20]; // 经度
        sprintf(latitude_str, "%f", gps_data.latitude);
        ui.draw_string(0, 120, latitude_str, &Font16);

        char longitude_str[20]; // 纬度
        sprintf(longitude_str, "%f", gps_data.longitude);
        ui.draw_string(0, 170, longitude_str, &Font16);

        char speed_str[20]; // 速度
        sprintf(speed_str, "%f", gps_data.speed_kmh);
        ui.draw_string(70, 60, speed_str, &Font16);

        char time_str[10]; // 时间
        time_str[0] = gps_data.hour / 10 + '0';
        time_str[1] = gps_data.hour % 10 + '0';
        time_str[2] = ':';
        time_str[3] = gps_data.minute / 10 + '0';
        time_str[4] = gps_data.minute % 10 + '0';
        time_str[5] = '\0';
        ui.draw_string(110, 120, time_str, &Font16);

        char pitch_str[20]; // 坡度
        sprintf(pitch_str, "%f", imu_data.pitch / 32768.0 * 180.0);
        ui.draw_string(110, 170, pitch_str, &Font16);

        ui.print_part(window_full);
        vTaskDelay(pdMS_TO_TICKS(2000));

        WINDOW clear_font1 = {
            .x_start = 0,
            .y_start = 120,
            .width = 90,
            .height = 16,
        };
        WINDOW clear_font2 = {
            .x_start = 0,
            .y_start = 170,
            .width = 90,
            .height = 16,
        };
        WINDOW clear_font3 = {
            .x_start = 70,
            .y_start = 60,
            .width = 90,
            .height = 16,
        };
        WINDOW clear_font4 = {
            .x_start = 110,
            .y_start = 120,
            .width = 90,
            .height = 16,
        };
        WINDOW clear_font5 = {
            .x_start = 110,
            .y_start = 170,
            .width = 90,
            .height = 16,
        };
        ui.clear_area(clear_font1);
        ui.clear_area(clear_font2);
        ui.clear_area(clear_font3);
        ui.clear_area(clear_font4);
        ui.clear_area(clear_font5);
    }
}
