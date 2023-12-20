#include "jy901.h"
 
// 创建一个JY901_DATA的实例
JY901_DATA jy901_data;

static const char *TAG = "UART"; //日志标签

//初始化串口 
void jy901_init() {
    //配置串口参数结构体
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE, //波特率
        .data_bits = UART_DATA_8_BITS, //数据位
        .parity = UART_PARITY_DISABLE, //无奇偶校验
        .stop_bits = UART_STOP_BITS_1, //停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE //无流控
    };
    //设置串口参数
    uart_param_config(UART_NUM, &uart_config);
    //安装串口驱动
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

//接收数据
void jy901_read() {
    uint8_t buffer[BUF_SIZE]; //接收缓冲区
    int len = 0; //接收数据长度
    uint8_t sumcrc = 0x55; //校验和
    uint8_t type = 0; //数据类型
    int16_t data1 = 0; //数据1
    int16_t data2 = 0; //数据2
    int16_t data3 = 0; //数据3
    int16_t data4 = 0; //数据4
    //读取数据
    len = uart_read_bytes(UART_NUM, buffer, BUF_SIZE, 100 / portTICK_RATE_MS);
    if (len > 0) {
        ESP_LOGI(TAG, "Receive %d bytes", len);
        // 遍历读取到的数据
        for (int i = 0; i < len;)
          { 
            //检查协议头
            if (buffer[i++] == 0x55) {
                //获取数据类型
                type = buffer[i++];
                sumcrc += type;
                //获取数据1
                data1 = (int16_t)((int16_t)buffer[i+1] << 8 | buffer[i]); //高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                //获取数据2
                data2 = (int16_t)((int16_t)buffer[i+1] << 8 | buffer[i]); //高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                //获取数据3
                data3 = (int16_t)((int16_t)buffer[i+1] << 8 | buffer[i]); //高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                //获取数据4
                data4 = (int16_t)((int16_t)buffer[i+1] << 8 | buffer[i]); //高低字节合并
                sumcrc += buffer[i++];
                sumcrc += buffer[i++];
                //检查校验和
                printf("%d ",sumcrc &  0x00ff);
                printf("%d*",buffer[i+1]);
                sumcrc = (char)sumcrc &  0x00ff; //取低八位
                if (1/*sumcrc == buffer[i++]*/) {
                    //ESP_LOGI(TAG, "Data valid");
                    //处理数据        
                    // 根据数据帧的第二个字节（功能字节）判断数据类型
                    switch (type)
                    {
                    case 0x51: // 加速度
                      // 将数据存储到jy901_data结构体中
                      jy901_data.accX = data1; // 读取X轴加速度
                      jy901_data.accY = data2; // 读取Y轴加速度
                      jy901_data.accZ = data3; // 读取Z轴加速度
                      break;
                    case 0x52: // 角速度
                      // 将数据存储到jy901_data结构体中
                      jy901_data.gyroX = data1; // 读取X轴角速度
                      jy901_data.gyroY = data2; // 读取Y轴角速度
                      jy901_data.gyroZ = data3; // 读取Z轴角速度
                      break;
                    case 0x53: // 角度
                      // 将数据存储到jy901_data结构体中
                      jy901_data.roll = data1; // 读取横滚角
                      jy901_data.pitch = data2; // 读取俯仰角
                      jy901_data.yaw = data3; // 读取偏航角
                      //printf(i);
                      break;
                    case 0x54: // 磁场
                      // 将数据存储到jy901_data结构体中
                      jy901_data.magX = data1; // 读取X轴磁场
                      jy901_data.magY = data2; // 读取Y轴磁场
                      jy901_data.magZ = data3; // 读取Z轴磁场
                      break;
                    default: // 其他类型
                      break;
                    }
                  }
                  else{
                    //ESP_LOGI(TAG, "Data invalid");
                  }
            }
          }
    }
    else {
      ESP_LOGW(TAG, "No data");
    }
}


// 打印JY901的数据
void jy901_print()
{
  // 打印数据到串口
  printf("Roll: "); // 打印横滚角
  printf("%f", jy901_data.roll / 32768.0 * 180.0); // 转换为角度
  printf("°\t");

  printf("Pitch: "); // 打印俯仰角
  printf("%f", jy901_data.pitch / 32768.0 * 180.0); // 转换为角度
  printf("°\t");

  printf("Yaw: "); // 打印偏航角
  printf("%f", jy901_data.yaw / 32768.0 * 180.0); // 转换为角度
  printf("°\n");
}
