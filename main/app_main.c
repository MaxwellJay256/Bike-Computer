/**
 * @file app_main.c
 * @brief Main application source code
*/
#include "app_main.h"
#include "mpu6050.h"

/// @brief Main application entry point
void app_main(void)
{
    mpu6050_output_t mpu6050_out;

    mpu6050_init();

    while (1)
    {
        mpu6050_out = mpu6050_get_value();
        /*/
        printf("accl_xout: %d\t", mpu6050_out.accel.x);
        printf("accl_yout: %d\t", mpu6050_out.accel.y);
        printf("accl_zout: %d\n", mpu6050_out.accel.z);
        //*/
        // printf("temp_out: %d\n;", mpu6050_out.temp);
        //*/
        printf("gyro_xout: %d\t", mpu6050_out.gyro.x);
        printf("gyro_yout: %d\t", mpu6050_out.gyro.y);
        printf("gyro_zout: %d\n", mpu6050_out.gyro.z);
        //*/
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    
}