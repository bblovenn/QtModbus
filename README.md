# Qt Modbus RTU/TCP 工业设备监控与调试系统

## 项目简介

本项目是一个基于 C++17 和 Qt Widgets 开发的工业上位机系统，面向 Modbus RTU/TCP 设备调试、周期采集、实时监控、报警处理和历史数据追溯场景。

它不是一个只做“读写寄存器”的小工具，而是围绕工业设备联调流程设计的一套桌面端监控系统。系统能够连接模拟设备或真实下位机，完成常用 Modbus 功能码调试、周期轮询采集、工程量转换、报警判断、SQLite 持久化、历史查询、报文日志记录和自动重连。

## 界面截图

> 将截图放到 `docs/images/` 目录后，替换下面的文件名即可。

### 主界面

![主界面](docs/images/main-window.png)

### 寄存器调试

![寄存器调试](docs/images/register-panel.png)

### 实时监控与曲线

![实时监控与曲线](docs/images/monitor-trend.png)

### 报警与历史记录

![报警与历史记录](docs/images/alarm-history.png)

## 功能总览

### 设备连接

系统支持两类 Modbus 通信方式：

- Modbus TCP
- Modbus RTU

TCP 模式支持配置：

```text
IP 地址
端口
从站地址
超时时间
重试次数
```

RTU 模式支持配置：

```text
串口号
波特率
数据位
停止位
校验位
从站地址
超时时间
重试次数
```

连接参数会通过 `QSettings` 持久化，程序重启后自动恢复上一次配置。

### 寄存器与线圈调试

支持常用 Modbus 功能码：

| 功能码 | 名称             | 支持情况 |
| ------ | ---------------- | -------- |
| 01     | 读取线圈         | 支持     |
| 02     | 读取离散输入     | 支持     |
| 03     | 读取保持寄存器   | 支持     |
| 04     | 读取输入寄存器   | 支持     |
| 05     | 写单个线圈       | 支持     |
| 06     | 写单个保持寄存器 | 支持     |
| 15     | 写多个线圈       | 支持     |
| 16     | 写多个保持寄存器 | 支持     |

说明：Modbus 功能码并不是连续按常用程度排列的。本项目覆盖工业调试中常用的 01/02/03/04/05/06/15/16，07-14 属于诊断、事件、通信记录等较少用于通用寄存器调试的功能码。

### 调试页面支持：

- 选择数据区
- 设置起始地址
- 设置读取数量
- 写单个保持寄存器
- 写单个线圈
- 批量写保持寄存器
- 批量写线圈
- 读取结果以十进制和十六进制形式显示

### 周期采集

连接成功后，系统默认周期读取保持寄存器地址 `0-3`，并将原始值转换为工程量。

默认数据映射：

```text
地址 0: 温度原始值，实际温度 = raw / 10.0
地址 1: 电压原始值，实际电压 = raw / 100.0
地址 2: 电流原始值，实际电流 = raw / 100.0
地址 3: 转速原始值，单位 rpm
```

采集参数支持配置：

```text
轮询周期
起始地址
读取数量
```

采集配置同样会通过 `QSettings` 保存。

### 实时监控

实时监控页面展示：

- 当前值
- 最小值
- 最大值
- 平均值
- 最新更新时间

监控对象包括：

```text
温度
电压
电流
转速
```

实时曲线页面用于观察数据变化趋势，适合演示设备状态随时间变化的效果。

### 报警系统

报警规则：

```text
温度超过上限: 严重报警
电压低于下限: 警告报警
连续通信失败: 设备离线报警
```

报警记录包含：

```text
报警 ID
报警时间
设备 ID
报警类型
报警等级
当前值
阈值
报警消息
是否确认
确认时间
```

报警支持：

- 实时报警显示
- 报警确认
- 报警历史查询
- 报警记录持久化

### SQLite 历史存储

系统使用 SQLite 保存三类核心数据：

```text
collect_data: 采集数据
alarm_log: 报警日志
packet_log: 报文日志
```

历史查询支持：

- 按设备 ID 查询
- 按时间范围查询
- 查询采集数据
- 查询报警记录
- 查询报文日志
- CSV 导出

### 报文日志

系统记录以下操作和事件：

- 连接请求
- 断开请求
- 寄存器读取请求
- 线圈读取请求
- 单写请求
- 批量写请求
- 读取成功响应
- 写入成功响应
- 通信错误
- 报警事件
- 自动重连事件

报文日志记录通信行为、操作结果和错误摘要，便于定位设备联调过程中的通信问题。

### 自动重连

系统支持自动重连配置：

```text
是否启用自动重连
重连间隔
```

主动点击断开连接不会触发自动重连；设备异常断开时，`ReconnectController` 会根据最近一次连接配置发起重连。

### Python 模拟器和 smoke test

项目提供 Python Modbus TCP 模拟器：

```text
simulator/tcp_text.py
```

还提供自动联调脚本：

```text
simulator/test.py
```

test 会自动验证：

- 01 读取线圈
- 02 读取离散输入
- 03 读取保持寄存器
- 04 读取输入寄存器
- 05 写单个线圈
- 06 写单个保持寄存器
- 15 写多个线圈
- 16 写多个保持寄存器

## 技术栈

| 分类   | 技术                        |
| ------ | --------------------------- |
| 语言   | C++17, Python 3             |
| GUI    | Qt Widgets                  |
| 通信   | Qt SerialBus, Qt SerialPort |
| 数据库 | Qt SQL, SQLite              |
| 构建   | CMake, Make                 |
| 模拟器 | pymodbus                    |

## 环境要求

推荐开发环境：

```text
Qt 6.x
Qt Creator
CMake 3.16+
C++17 编译器
Python 3.9+
pymodbus 3.6.9
```

安装 Python 依赖：

```bash
cd <你的项目根目录>/QtModbus
python3 -m pip install -r simulator/requirements.txt
```

## 目录结构

```text
QtModbus/
├── CMakeLists.txt
├── README.md
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── core/
│   ├── DeviceConfig.h
│   ├── RegisterValue.h
│   ├── EngineeringValue.h
│   ├── AlarmRecord.h
│   └── PacketRecord.h
├── communication/
│   ├── IModbusClient.h
│   ├── QtModbusClient.h
│   ├── QtModbusClient.cpp
│   ├── ReconnectController.h
│   └── ReconnectController.cpp
├── polling/
│   ├── PollingWorker.h
│   └── PollingWorker.cpp
├── alarm/
│   ├── AlarmManager.h
│   └── AlarmManager.cpp
├── database/
│   ├── DatabaseManager.h
│   └── DatabaseManager.cpp
├── ui/
│   ├── ConnectionPanel.*
│   ├── RegisterPanel.*
│   ├── MonitorPanel.*
│   ├── TrendPanel.*
│   ├── AlarmPanel.*
│   ├── AlarmConfigPanel.*
│   ├── HistoryPanel.*
│   ├── PacketMonitorPanel.*
│   ├── PacketHistoryPanel.*
│   ├── DatabaseMaintenancePanel.*
│   ├── PollingConfigPanel.*
│   ├── ReconnectConfigPanel.*
│   └── SystemStatusPanel.*
├── simulator/
│   ├── tcp_text.py
│   ├── test.py
│   └── requirements.txt
└── docs/
    └── demo_script.md
```

## 构建项目

默认推荐在 Qt Creator 中构建项目：

```text
1. 打开 Qt Creator
2. 打开 <你的项目根目录>/QtModbus/CMakeLists.txt
3. 选择可用的 Qt Kit
4. 点击构建
```

构建成功时，Qt Creator 的编译输出中应看到：

```text
[100%] Built target Modbus
```

项目主构建方式是 CMake，同时提供 `Modbus.pro` 兼容 qmake。

如果需要命令行备用构建，可以使用 Qt Creator 生成的构建目录：

```bash
cmake --build <Qt Creator 生成的构建目录> --target Modbus
```

如果你的环境没有 `cmake` 命令，也可以进入 Qt Creator 生成的构建目录后使用 `make`：

```bash
make
```

如果需要使用 qmake，可以在 Qt 的 qmake 命令可用时执行：

```bash
cd <你的项目根目录>/QtModbus
qmake Modbus.pro
make
```

如果 shell 找不到 `qmake`，可以使用你本机 Qt 安装目录中的 qmake，例如：

```bash
<你的Qt安装目录>/bin/qmake Modbus.pro
make
```

## 快速开始

```text
1. 使用 Qt Creator 打开 QtModbus/CMakeLists.txt
2. 选择 Qt Kit 并构建 Modbus 目标
3. 启动 Python 模拟器
4. 运行 smoke test 确认协议功能
5. 从 Qt Creator 运行上位机
6. 使用默认连接参数连接 127.0.0.1:5020
```

## 启动模拟器

```bash
cd <你的项目根目录>/QtModbus
python3 simulator/tcp_text.py
```

默认监听：

```text
127.0.0.1:5020
```

如果提示端口已占用，说明已有模拟器或其他程序正在监听 `5020`，需要先关闭旧进程。

## 运行 smoke test

模拟器启动后，另开终端执行：

```bash
cd <你的项目根目录>/QtModbus
python3 simulator/test.py
```

成功时输出：

```text
All tests passed.
```

## 启动上位机

默认推荐在 Qt Creator 中运行程序：

```text
1. 打开 Qt Creator
2. 打开 <你的项目根目录>/QtModbus/CMakeLists.txt
3. 选择可用的 Qt Kit
4. 选择 Modbus 目标
5. 点击运行
```

如果需要命令行备用启动，可以运行 Qt Creator 构建目录中的 `Modbus` 可执行文件：

```bash
<Qt Creator 生成的构建目录>/Modbus
```

## 默认连接参数

```text
模式: Modbus TCP
IP: 127.0.0.1
端口: 5020
从站地址: 1
超时: 1000 ms
重试次数: 3
```

## 架构说明

系统采用分层和信号槽解耦设计：

```text
UI 层
  ConnectionPanel / RegisterPanel / MonitorPanel / AlarmPanel / HistoryPanel

业务层
  PollingWorker / AlarmManager / ReconnectController

通信层
  IModbusClient / QtModbusClient

数据层
  DatabaseManager / SQLite
```

核心思路：

- UI 层只表达用户操作和显示结果，不直接操作底层 Modbus 对象。
- 通信层通过 `IModbusClient` 抽象，实际实现由 `QtModbusClient` 完成。
- 周期采集通过 `PollingWorker` 发起读请求。
- 报警判断由 `AlarmManager` 独立处理。
- 数据库存储由 `DatabaseManager` 统一负责。
- 模块之间主要通过 Qt 信号槽通信，降低耦合。

## License

本项目基于 MIT License 开源，详见 [LICENSE](LICENSE)。
