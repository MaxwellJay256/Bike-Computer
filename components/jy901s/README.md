# JY901S

> by [@chickencheng](https://github.com/chickencheng), 2023.12

## example
```c
// 主函数
void app_main()
{
  printf("1");
  // 初始化UART
  jy901s_init();
  printf("2");
  // 循环执行
  while (1)
  {
    printf("1");
    // 读取JY901的数据
    jy901s_read();
    // 打印JY901的数据
    jy901s_print();
    // 延时100毫秒
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
```