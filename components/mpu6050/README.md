# mpu6050

This component is a driver for the MPU6050 6-axis motion tracking device.

Author: [@MaxwellJay256](https://github.com/MaxwellJay256)

Version: 2.0

> Reference: [*MPU6050陀螺仪驱动代码移植，ESP-IDF，ESP32 - CSDN*](http://t.csdnimg.cn/3rzi2)

## Content

This component only has two source files, plus a `CMakeLists.txt` file.
- [mpu6050.h](./mpu6050.h): The header file of the driver.
- [mpu6050.c](./mpu6050.c): The source file of the driver.
- [CMakeLists.txt](./CMakeLists.txt): A CMake file registering this component.

## Usage

### Edit the variables and macros

There are 4 macros you must notice in [`mpu6050.h`](./mpu6050.h).
You should edit them to fit your hardware.
The default values are suitable for `ESP32-DevKitC`.

```c
#define MPU6050_I2C_SDA 19             // MPU6050 I2C 的 SDA 引脚
#define MPU6050_I2C_SCL 18             // MPU6050 I2C 的 SCL 引脚
#define MPU6050_I2C_FREQ 400000        // MPU6050 I2C 的频率
#define MPU6050_I2C_PORT_NUM I2C_NUM_0 // MPU6050 的 I2C 端口号
```

### Example

This is an example of running this component which simply
reads the data from the MPU6050 and prints it to the serial port.

```c
#include "mpu6050.h"

void app_main(void)
{
    mpu6050_output_t mpu6050_out;
    mpu6050_config_t mpu6050_config = {
        .gyro_fsr = 3,
        .accel_fsr = 0,
        .sample_rate = 100,
    };

    mpu6050_init(mpu6050_config);

    while (1)
    {
        mpu6050_out = mpu6050_get_value();
        //*/
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
        vTaskDelay(pdMS_TO_TICKS(200));
    }    
}
```

If the initialization is successful, the log will be like this:

```
I (323) MPU6050: MPU6050 Initializing...
I (333) MPU6050: i2c_param_config: ESP_OK
I (333) MPU6050: i2c_driver_install: ESP_OK
I (433) MPU6050: Set gyroscopes full-scale range to ±2000 °/s.
I (433) MPU6050: Set accelerometers full-scale range to ±2g.
I (433) MPU6050: Set sampling rate to 50 Hz.
I (433) MPU6050: Set low pass filter to 25 Hz.
I (443) MPU6050: Read mpu6050_device_id: 0x68
I (443) MPU6050: Set sampling rate to 50 Hz.
I (453) MPU6050: Set low pass filter to 25 Hz.
I (453) MPU6050: Init Success.
```