# Bike-Computer

HITSZ 2023 大一立项项目——自行车 GPS 码表

Collaborators: [chaos](https://github.com/chaos0514), [Maxwell Jay](https://github.com/MaxwellJay256), [CY](https://github.com/CYCLECHENHUOHUA), [chickencheng](https://github.com/chickencheng)

<img src="https://github.com/MaxwellJay256/Bike-Computer/assets/107470012/be9018b6-b457-4461-9c04-e241b6ffcd37" width=500 align="center">

项目基于 `ESP32`，使用 `ESP-IDF v4.4.6` C 语言框架。

## Contents

- [Hardware](./Hardware)：硬件设计文件
- [components](./components)：ESP-IDF 组件，相当于几个独立的库
  - [mpu6050](./components/mpu6050)：MPU6050 6 轴陀螺仪驱动（已弃用）
  - [epaper](https://github.com/MaxwellJay256/GDEY0154D67-esp32-driver)：电子墨水屏 GDEY0154D67 的驱动，以一个单独的仓库托管，在本项目中作为一个 submodule 引用。
  - [jy901s](./components/jy901s)：JY901S 9 轴陀螺仪驱动
  - [gps-ATGM336H](./components/gps-ATGM336H)：ATGM336H GPS 模块驱动
- [main](./main)：项目的主程序代码。

## License

本项目使用 MIT 协议开源。
