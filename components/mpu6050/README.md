# mpu6050

This component is a driver for the MPU6050 6-axis motion tracking device.

Author: [@MaxwellJay256](https://github.com/MaxwellJay256)

Version: 1.0

> Reference: [*esp32系列(10)：mpu6050驱动 - CSDN*](http://t.csdnimg.cn/vPGnV)

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

Next, in [`mpu6050.c`](./mpu6050.c), you should pay attention to the `mpu6050_init_cmd` structure, which provides the initialization commands for the MPU6050.

```c
static uint8_t mpu6050_init_cmd[MPU6050_INIT_CMD_COUNT][2] = {
    // format: {address, data}
    {0x6B, 0x80}, // PWR_MGMT_1, DEVICE_RESET
    {0x6B, 0x00}, // clear SLEEP
    {0x1B, 0x18}, // Gyroscope Full Scale Range = ± 2000 °/s
    {0x1c, 0x00}, // Accelerometer Full Scale Range = ± 2g
    {0x38, 0x00}, // Disable Interrupt
    {0x6A, 0x00}, // User Control.auxiliary I2C are logically driven by the primary I2C bus
    {0x23, 0x00}, // FIFO Enable.disable
    {0x19, 0x63}, // Sample Rate Divider.Sample Rate = 1 kHz / (1 + 99) = 10 Hz
    {0x1A, 0x13}, // EXT_SYNC_SET = GYRO_XOUT_L[0]; Bandwidth = 3
    {0x6B, 0x01}, // Power Management 1.PLL with X axis gyroscope reference
    {0x6C, 0x00}, // Power Management 2
};
```

### Call the APIs

Only two functions are provided in this component.

#### `esp_err_t mpu6050_init()`

This function initializes the MPU6050, with the macros and variables you have edited above.

#### `mpu6050_output_t mpu6050_get_value()`

This function reads the data from the MPU6050, and returns a `mpu6050_output_t` structure.

The structure is defined as follows:

```c
typedef struct {
    struct {
        int16_t x; int16_t y; int16_t z;
    } accel;
    int16_t temp;
    struct {
        int16_t x; int16_t y; int16_t z;
    } gyro;
} mpu6050_output_t;
```

### Example

This is an example of running this component which simply
reads the data from the MPU6050 and prints it to the serial port.

```c
#include "mpu6050.h"

void app_main(void)
{
    mpu6050_output_t mpu6050_out;
    mpu6050_init();

    while (1)
    {
        mpu6050_out = mpu6050_get_value();
        
        printf("accl_xout: %d\t", mpu6050_out.accel.x);
        printf("accl_yout: %d\t", mpu6050_out.accel.y);
        printf("accl_zout: %d\n", mpu6050_out.accel.z);
        
        // printf("temp_out: %d\n;", mpu6050_out.temp);
        
        printf("gyro_xout: %d\t", mpu6050_out.gyro.x);
        printf("gyro_yout: %d\t", mpu6050_out.gyro.y);
        printf("gyro_zout: %d\n", mpu6050_out.gyro.z);
        
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}