# EZBOOT

## 介绍

EZBOOT 是一款专为 32 位ARM微控制器设计的引导加载程序。它定义了一套通用的引导加载程序基础架构和系统闪存布局，提供了便捷的软件升级功能。EZBOOT 独立于特定的操作系统和硬件，具备高度的灵活性与适应性。

## 特点

- 资源占用低，ROM最小占用可达6K，RAM最小占用可达5K（支持OTA）


- 安全，支持对OTA固件加密


- 支持log打印，方便调试

- 代码简单，移植方便


## 代码目录

components/boot - 引导程序进入app

components/crypt - 加密算法库

components/ezb_flash - 封装片内flash的擦除、读取、写入等接口

components/mlog - 日志系统

components/norflash - 支持对片外norflash的擦除、读取、写入

components/ota - 实现OTA的完整流程

components/ota_mgr - OTA标识符管理

project - 支持的板型或芯片的工程样例

script - OTA文件生成脚本

## 使用方法

[EZBOOT移植](doc/PORTING.md)