# STM32 HAL 驱动库

## 项目简介

本项目用于整理和汇总基于 STM32 HAL 库开发的各类外设及器件驱动，同时记录本人在 STM32 学习与实践过程中的经验和成果。

部分驱动的详细配置与实现过程将在个人博客中持续更新。

[查看相关博客](https://fazhehy.github.io/archive/?category=embedded)

## CHANGELOG

### 2026-09-09

#### [./STM32H747/screen](./STM32H747/screen)
- 新增 STM32H747 KD025EGOIN001 400×712 AMOLED 工程，完成 LTDC、MIPI DSI、DMA2D、FMC SDRAM 及时钟配置。
- 新增 KD025EGOIN001 屏幕驱动，包含 RM690C0 初始化、SGM3836A 偏压控制、DSI 命令发送、DMA2D 图形绘制和测试画面。
- 完成单 Lane 600Mbit/s、Burst Mode、18.1875MHz Pixel Clock 配置，并补充 MIPI DSI 原理与 CubeMX 配置文档。

### 2026-09-01

#### [./STM32H747/sdram](./STM32H747/sdram)
- 新增 STM32H747 IS42S32800J 32MB SDRAM 工程，完成 FMC Bank2、PLL2R 220MHz、SDCLK 110MHz 及时序配置。
- 新增 SDRAM 初始化与读写测试驱动，包含 JEDEC 上电初始化、自动刷新、模式寄存器配置、数据线测试、地址线测试和 1MB 连续读写测试。
- SDRAM 驱动与 MPU 配置保持独立，并提供初始化测试、指针访问、帧缓冲和外部堆使用示例。

### 2026-08-31

#### [./STM32H747/mpu](./STM32H747/mpu)
- 新增 STM32H747 CM7 MPU 内存保护工程，包含内存区域配置、只读访问保护实验、MemManage Fault 异常处理以及 MPU 与 Cache 学习文档。

### 2026-08-30 

#### [./STM32H747/template](./STM32H747/template)
- 新增 STM32H747 双核工程模板，包含基础工程结构、日志输出、延时函数以及编译烧录脚本。
