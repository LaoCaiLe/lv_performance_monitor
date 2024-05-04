# performance_monitor_base_lvgl

基于lvgl开源库实现的linux性能监测程序

![PixPin_2024-04-30_00-19-00](https://github.com/LaoCaiLe/lv_performance_monitor/assets/70246846/beeadfee-e3da-4696-8135-2fe6329ed695)

## Install SDL
```
编译master分支程序需要添加sdl环境
sudo apt-get update && sudo apt-get install -y build-essential libsdl2-dev
```

## Clone
```
git clone https://github.com/LaoCaiLe/lv_performance_monitor/
cd lv_performance_monitor/
git submodule update --init --recursive [--depth=1]
```

## Build
```
cd lv_performance_monitor/
make -j
```

## Run
**master**:
```
./demo
```
**on-board**:
```
确保已添加/dev/fb0设备
sudo adduser $USER video 设置用户为video组
./demo
```
## repo_info
```
master分支： forked from https://github.com/lvgl/lv_port_pc_vscode -> 用于ubuntu系统
on-board分支： forked from https://github.com/lvgl/lv_port_linux_frame_buffer -> 用于linux开发板
lvgl版本：v8.3.9
```
