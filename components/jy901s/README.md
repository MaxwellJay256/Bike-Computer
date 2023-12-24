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

## Reference

- [JY901S产品资料 - 维特智能](https://wit-motion.yuque.com/wumwnr/docs/khbgzd?singleDoc#%20%E3%80%8AJY901S%E4%BA%A7%E5%93%81%E8%B5%84%E6%96%99%E3%80%8B)