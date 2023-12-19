# Bike-Computer

HITSZ 2023 大一立项项目——自行车 GPS 码表

Collaborators: [chaos](https://github.com/chaos0514), [Maxwell Jay](https://github.com/MaxwellJay256), [CY](https://github.com/CYCLECHENHUOHUA), [chickencheng](https://github.com/chickencheng)

项目基于 `ESP32`，使用 `ESP-IDF v4.4.6` C 语言框架。

## Contents

- [Hardware](./Hardware)：硬件设计（电路原理图、PCB设计、元器件清单、外壳工程图等）的文档。
- [components](./components)：项目组件
  - [mpu6050](./components/mpu6050)：MPU6050 6轴陀螺仪驱动（已弃用）
  - [epaper](https://github.com/MaxwellJay256/GDEY0154D67-esp32-driver)：电子墨水屏 GDEY0154D67 的驱动，以一个单独的仓库托管，在本项目中作为一个 submodule 引用。
  - [jy901s](./components/jy901s)：JY901S 9轴陀螺仪驱动（待开发）
- [main](./main)：项目的主要源代码。

## License

本项目使用 MIT 协议开源，详见 [LICENSE](./LICENSE) 文件。