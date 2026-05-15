# Qt Modbus RTU/TCP 工业设备监控与调试系统

## 项目简介

本项目是一个基于 C++17 和 Qt Widgets 开发的工业上位机系统，面向 Modbus RTU/TCP 设备调试、周期采集、实时监控、报警处理和历史数据追溯场景。

它不是一个只做“读写寄存器”的小工具，而是围绕工业设备联调流程设计的一套桌面端监控系统。系统能够连接模拟设备或真实下位机，完成常用 Modbus 功能码调试、周期轮询采集、工程量转换、报警判断、SQLite 持久化、历史查询、报文日志记录和自动重连。

项目适合用于展示以下能力：

- Qt Widgets 桌面应用开发
- Qt 信号槽解耦设计
- Modbus TCP / RTU 工业协议调试
- 串口通信和 TCP 通信基础
- 数据采集、报警判断、历史存储
- SQLite 本地数据库设计
- Python 模拟下位机联调
- 工业上位机项目工程组织能力

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

当前支持常用 Modbus 功能码：

| 功能码 | 名称 | 当前支持 |
| --- | --- | --- |
| 01 | 读取线圈 | 支持 |
| 02 | 读取离散输入 | 支持 |
| 03 | 读取保持寄存器 | 支持 |
| 04 | 读取输入寄存器 | 支持 |
| 05 | 写单个线圈 | 支持 |
| 06 | 写单个保持寄存器 | 支持 |
| 15 | 写多个线圈 | 支持 |
| 16 | 写多个保持寄存器 | 支持 |

调试页面支持：

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

当前实现基础报警规则：

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

当前报文日志记录的是通信行为和结果摘要，后续可扩展为真实十六进制 TX/RX 原始报文。

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
simulator/modbus_tcp_server.py
```

还提供自动联调脚本：

```text
simulator/modbus_smoke_test.py
```

smoke test 会自动验证：

- 01 读取线圈
- 02 读取离散输入
- 03 读取保持寄存器
- 04 读取输入寄存器
- 05 写单个线圈
- 06 写单个保持寄存器
- 15 写多个线圈
- 16 写多个保持寄存器

## 技术栈

| 分类 | 技术 |
| --- | --- |
| 语言 | C++17, Python 3 |
| GUI | Qt Widgets |
| 通信 | Qt SerialBus, Qt SerialPort |
| 数据库 | Qt SQL, SQLite |
| 构建 | CMake, Make |
| 模拟器 | pymodbus |

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
│   ├── modbus_tcp_server.py
│   ├── modbus_smoke_test.py
│   └── requirements.txt
└── docs/
    └── demo_script.md
```

## 构建项目

在项目根目录执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus
make -C build-vscode-intellisense
```

构建成功时输出：

```text
[100%] Built target Modbus
```

项目主构建方式是 CMake，同时提供 `Modbus.pro` 兼容 qmake。

如果需要使用 qmake，可以在 Qt 的 qmake 命令可用时执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus/QtModbus
qmake Modbus.pro
make
```

如果 shell 找不到 `qmake`，可以使用 Qt 安装目录中的 qmake，例如：

```bash
/Users/bbnn/Qt/6.10.3/macos/bin/qmake Modbus.pro
make
```

## 启动模拟器

```bash
cd /Users/bbnn/Documents/QtProject/modbus/QtModbus
python3 simulator/modbus_tcp_server.py
```

默认监听：

```text
127.0.0.1:5020
```

如果提示端口已占用，说明已有模拟器或其他程序正在监听 `5020`，需要先关闭旧进程。

## 运行 smoke test

模拟器启动后，另开终端执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus/QtModbus
python3 simulator/modbus_smoke_test.py
```

成功时输出：

```text
All Modbus smoke tests passed.
```

## 启动上位机

```bash
cd /Users/bbnn/Documents/QtProject/modbus
./build-vscode-intellisense/Modbus
```

也可以直接从 Qt Creator 或 IDE 中运行 `Modbus` 目标程序。

## 默认连接参数

```text
模式: Modbus TCP
IP: 127.0.0.1
端口: 5020
从站地址: 1
超时: 1000 ms
重试次数: 3
```

## 演示流程

推荐演示顺序：

```text
1. 启动 Python 模拟器
2. 运行 smoke test，证明协议功能可用
3. 启动 Qt 上位机
4. 连接 127.0.0.1:5020
5. 演示寄存器和线圈读写
6. 演示周期采集和实时曲线
7. 演示报警触发和报警确认
8. 演示历史查询和 CSV 导出
9. 演示报文日志
10. 演示自动重连
```

更详细的演示说明见：

```text
docs/demo_script.md
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

## 面试讲解要点

可以这样介绍项目：

```text
这是一个基于 Qt 的工业上位机项目，目标是实现 Modbus 设备的连接、调试、采集、报警和历史追溯。

我没有让 UI 直接调用 QModbusTcpClient，而是封装了 IModbusClient 接口和 QtModbusClient 实现。这样 UI 层只关心读写请求和结果信号，后续如果替换通信实现或增加模拟客户端，改动范围会更小。

采集部分使用 PollingWorker 定时读取保持寄存器，并把原始寄存器值转换成温度、电压、电流、转速等工程量。工程量通过信号发送给实时监控、趋势曲线、报警管理和数据库模块。

报警部分独立封装在 AlarmManager 中，负责判断温度高限、电压低限和设备离线等条件。报警产生后会显示到 UI，并保存到 SQLite。

项目还提供 Python Modbus TCP 模拟器和 smoke test，可以独立验证 01/02/03/04/05/06/15/16 常用功能码，方便演示和联调。
```

## 当前验证结果

当前项目已验证：

```text
Qt 工程可编译
Python 模拟器可启动
smoke test 可通过
上位机可连接模拟器
常用 Modbus 功能码读写可用
采集、报警、历史、日志、重连功能已接入
```

## 后续可扩展方向

### 寄存器点表配置

将温度、电压、电流、转速这些硬编码映射改为可配置点表：

```text
名称
寄存器类型
起始地址
数据类型
倍率
单位
报警上限
报警下限
是否启用
```

这样可以更接近真实工业设备说明书的配置方式。

### 通信任务队列

当前手动调试和周期轮询都可能发起 Modbus 请求。后续可以增加请求队列：

```text
手动调试请求优先
周期轮询请求排队
同一时间只处理一个 Modbus 请求
请求超时后继续处理下一条
```

### 原始报文日志

当前日志记录的是操作摘要，后续可扩展真实协议报文：

```text
TX: 十六进制请求报文
RX: 十六进制响应报文
异常码解析
CRC / MBAP 信息展示
```

### 数据库 Worker 线程

高频采集时可将数据库写入迁移到 Worker 线程，避免 UI 线程压力过大。

### 报表导出

后续可增加：

```text
报警报表导出
采集数据日报
设备运行统计
通信故障统计
```
