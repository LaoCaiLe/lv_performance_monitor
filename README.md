# performance_monitor_base_lvgl

基于lvgl开源库实现的linux性能监测程序

![PixPin_2024-04-30_00-19-00](https://github.com/LaoCaiLe/lv_performance_monitor/assets/70246846/beeadfee-e3da-4696-8135-2fe6329ed695)

## repo_info
```
lvgl版本：v8.3.9
master分支： forked from https://github.com/lvgl/lv_port_pc_vscode -> 用于ubuntu系统
on-board分支： forked from https://github.com/lvgl/lv_port_linux_frame_buffer -> 用于linux开发板
```

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

## transplant
移植本代码，只需要将工程中的lv_monitor文件夹拷贝到lvgl工程根目录下，在main函数中添加相关函数即可
```
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
...
#include "monitor.h" //+++

int main(void)
{
    lv_init();

    ...

    lv_monitor_show(); //+++

    while(1) {
        ...
    }

    return 0;
}

```