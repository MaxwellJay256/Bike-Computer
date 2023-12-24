## 配置环境
get_idf 

## 配置 
idf.py menuconfig

## 编译
idf.py build

## 清除
idf.py fullclean

## 下载
idf.py -p /dev/ttyUSB0 flash

## 监视器
idf.py -p /dev/ttyUSB0 monitor

## 构建、下载、监视
idf.py -p /dev/ttyUSB0 flash monitor

## 擦除
idf.py -p /dev/ttyUSB0 erase-flash

## 编译成功提示
Project build complete.

## 字体库下载
python esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x15D000 myFont1.bin

get_idf
cd Bike-Computer-main
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
 
git config --global --unset http.proxy 
git config --global --unset https.proxy
