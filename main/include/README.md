# include

This directory contains all the header files for the project.

## List of header files

- [`app_main.h`](app_main.h): 主头文件，包含所有其余的头文件，以及一些全局变量和宏定义。仅由 [`app_main.c`](../app_main.c) 引用。
- [hal](./hal)：硬件抽象层，所有与硬件相关的头文件，包括 GPIO、I2C、SPI 等。
- [gps](./gps): GPS相关的头文件
- ...