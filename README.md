# EZBOOT

## 介绍

EZBOOT 是一款专为 32 位ARM微控制器设计的引导加载程序。它定义了一套通用的引导加载程序基础架构和系统闪存布局，提供了便捷的软件升级功能。EZBOOT 独立于特定的操作系统和硬件，具备高度的灵活性与适应性。

## 特点

- 资源占用低，ROM最小占用可达6K，RAM最小占用可达2K（支持OTA）


- 安全，支持对OTA固件加密


- 支持log打印，方便调试
- 代码简单，移植方便
- 支持串口编程


## 代码目录

components/boot - 引导程序进入app

components/crypt - 加密算法库

components/ezb_flash - 封装片内flash的擦除、读取、写入等接口

components/mlog - 日志系统

components/norflash - 支持对片外norflash的擦除、读取、写入

components/ota - 实现OTA的完整流程

components/ota_mgr - OTA标识符管理

components/programming - 编程功能

components/ringbuffer - 环形缓冲区

components/sys - 系统工具接口

components/tlv - TLV数据协议接口

project - 支持的板型或芯片的工程样例

script - OTA文件生成脚本

## 使用方法

首先确保电脑已安装 Python，然依赖模块，在script文件下打开终端窗口，执行`pip install -r requirements.txt`。

[EZBOOT移植](doc/PORTING.md)

[APP适配指南](doc/APP.md)

[开启编程功能](doc/PROGRAMMING.md)

