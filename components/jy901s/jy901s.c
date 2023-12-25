/**
 * @file jy901s.c
 * @author @chickencheng
 */
#include "jy901s.h"

//JY901S_DATA jy901s_data;

static const char *TAG = "JY901S"; // 日志标签

// 初始化串口
void jy901s_init()
{
    // 配置串口参数结构体
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,               // 波特率
        .data_bits = UART_DATA_8_BITS,        // 数据位
        .parity = UART_PARITY_DISABLE,        // 无奇偶校验
        .stop_bits = UART_STOP_BITS_1,        // 停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE // 无流控
    };
    // 设置串口参数
    uart_param_config(UART_NUM, &uart_config);
    // 安装串口驱动
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// 接收处理数据，返回JY901S_DATA结构体
JY901S_DATA jy901s_read(JY901S_DATA jy901s_data)
{
    uint8_t buffer[BUF_SIZE]; // 接收缓冲区
    int len = 0;              // 接收数据长度
    uint8_t sumcrc = 0x55;    // 校验和
    uint8_t type = 0;         // 数据类型
    int16_t data1 = 0;        // 数据1
    int16_t data2 = 0;        // 数据2
    int16_t data3 = 0;        // 数据3
    int16_t data4 = 0;        // 数据4
    // 读取数据
    len = uart_read_bytes(UART_NUM, buffer, BUF_SIZE, 100 / portTICK_RATE_MS);
    if (len > 0)
    {
        ESP_LOGI(TAG, "Receive %d bytes", len);
        // 遍历读取到的数据
        for (int i = 0; i < len;)
        {
            // 检查协议头
            if (buffer[i++] == 0x55)
            {
                // 获取数据类型
                type = buffer[i++];
                sumcrc += type;
                // 获取数据1
                data1 = (int16_t)((int16_t)buffer[i + 1] << 8 | buffer[i]); // 高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                // 获取数据2
                data2 = (int16_t)((int16_t)buffer[i + 1] << 8 | buffer[i]); // 高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                // 获取数据3
                data3 = (int16_t)((int16_t)buffer[i + 1] << 8 | buffer[i]); // 高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                // 获取数据4
                data4 = (int16_t)((int16_t)buffer[i + 1] << 8 | buffer[i]); // 高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                // 检查校验和
                printf("%d ", sumcrc & 0x00ff);
                printf("%d*", buffer[i + 1]);
                sumcrc = (char)sumcrc & 0x00ff; // 取低八位
                if (1 /*sumcrc == buffer[i++]*/)
                {
                    // ESP_LOGI(TAG, "Data valid");
                    // 处理数据
                    // 根据数据帧的第二个字节（功能字节）判断数据类型
                    switch (type)
                    {
                    case 0x51: // 加速度
                        // 将数据存储到jy901_data结构体中
                        jy901s_data.accX = data1/32768.0*16*g; // 读取X轴加速度
                        jy901s_data.accY = data2/32768.0*16*g; // 读取Y轴加速度
                        jy901s_data.accZ = data3/32768.0*16*g; // 读取Z轴加速度
                        break;
                    case 0x52: // 角速度
                        // 将数据存储到jy901_data结构体中
                        jy901s_data.gyroX = data1/32768.0*2000; // 读取X轴角速度
                        jy901s_data.gyroY = data2/32768.0*2000; // 读取Y轴角速度
                        jy901s_data.gyroZ = data3/32768.0*2000; // 读取Z轴角速度
                        break;
                    case 0x53: // 角度
                        // 将数据存储到jy901_data结构体中
                        jy901s_data.roll  = data1/ 32768.0 * 180.0;  // 读取横滚角
                        jy901s_data.pitch = data2/ 32768.0 * 180.0;  // 读取俯仰角
                        jy901s_data.yaw   = data3/ 32768.0 * 180.0;  // 读取偏航角
                        // printf(i);
                        break;
                    case 0x54: // 磁场
                        // 将数据存储到jy901_data结构体中
                        jy901s_data.magX = data1; // 读取X轴磁场
                        jy901s_data.magY = data2; // 读取Y轴磁场
                        jy901s_data.magZ = data3; // 读取Z轴磁场
                        break;
                    default: // 其他类型
                        break;
                    }
                }
                else
                {
                    // ESP_LOGI(TAG, "Data invalid");
                }
            }
        }
    }
    else
    {
        ESP_LOGW(TAG, "No data");
    }
    return jy901s_data;
}

// 打印JY901的所有数据到串口
void jy901s_printall(JY901S_DATA jy901s_data)
{
    // 打印数据到串口
    //角度
    printf("Roll: ");               // 打印横滚角
    printf("%f", jy901s_data.roll); // 转换为角度
    printf("°\t");

    printf("Pitch: ");               // 打印俯仰角
    printf("%f", jy901s_data.pitch); // 转换为角度
    printf("°\t");

    printf("Yaw: ");                 // 打印偏航角
    printf("%f", jy901s_data.yaw);   // 转换为角度
    printf("°\n");

    //加速度
    printf("accX: ");                // 打印accX
    printf("%f", jy901s_data.accX);  // 转换为加速度
    printf("m/s^2\t");

    printf("accY: ");                 // 打印accY
    printf("%f", jy901s_data.accY);   // 转换为加速度
    printf("m/s^2\t");

    printf("accZ: ");                 // 打印accZ
    printf("%f", jy901s_data.accZ);   // 转换为加速度
    printf("m/s^2\n");
    
    //角速度
    printf("gyroX: ");                // 打印X
    printf("%f", jy901s_data.gyroX);  // 转换为角速度
    printf("°/s\t");

    printf("gyroY: ");                // 打印Y
    printf("%f", jy901s_data.gyroY);  // 转换为角速度
    printf("°/s\t");

    printf("gyroZ: ");                // 打印Z
    printf("%f", jy901s_data.gyroZ);  // 转换为角速度
    printf("°/s\n");

    //磁场
    printf("magX: ");              // 打印X
    printf("%f", jy901s_data.magX);  
    printf("mGs\t");

    printf("magY: ");             // 打印Y
    printf("%f", jy901s_data.magY); 
    printf("mGs\t");

    printf("magZ: ");             // 打印Z
    printf("%f", jy901s_data.magZ);   
    printf("mGs\n");
}

// 打印JY901的角度数据到串口
void jy901s_printangle(JY901S_DATA jy901s_data)
{
    //角度
    printf("Roll: ");               // 打印横滚角
    printf("%f", jy901s_data.roll); // 转换为角度
    printf("°\t");

    printf("Pitch: ");               // 打印俯仰角
    printf("%f", jy901s_data.pitch); // 转换为角度
    printf("°\t");

    printf("Yaw: ");                 // 打印偏航角
    printf("%f", jy901s_data.yaw);   // 转换为角度
    printf("°\n");
}

// 打印JY901的角速度数据到串口
void jy901s_printgyro(JY901S_DATA jy901s_data)
{
    //角速度
    printf("gyroX: ");                // 打印X
    printf("%f", jy901s_data.gyroX);  // 转换为角速度
    printf("°/s\t");

    printf("gyroY: ");                // 打印Y
    printf("%f", jy901s_data.gyroY);  // 转换为角速度
    printf("°/s\t");

    printf("gyroZ: ");                // 打印Z
    printf("%f", jy901s_data.gyroZ);  // 转换为角速度
    printf("°/s\n");
}

// 打印JY901的加速度数据到串口
void jy901s_printacc(JY901S_DATA jy901s_data)
{
    //加速度
    printf("accX: ");                // 打印accX
    printf("%f", jy901s_data.accX);  // 转换为加速度
    printf("m/s^2\t");

    printf("accY: ");                 // 打印accY
    printf("%f", jy901s_data.accY);   // 转换为加速度
    printf("m/s^2\t");

    printf("accZ: ");                 // 打印accZ
    printf("%f", jy901s_data.accZ);   // 转换为加速度
    printf("m/s^2\n");
}

// 打印JY901的磁场数据到串口
void jy901s_printmag(JY901S_DATA jy901s_data)
{
    //磁场
    printf("magX: ");              // 打印X
    printf("%f", jy901s_data.magX);  
    printf("mGs\t");

    printf("magY: ");             // 打印Y
    printf("%f", jy901s_data.magY); 
    printf("mGs\t");

    printf("magZ: ");             // 打印Z
    printf("%f", jy901s_data.magZ);   
    printf("mGs\n");
}
