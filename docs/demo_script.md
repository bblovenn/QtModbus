# 演示脚本

本文档用于完整演示 Qt Modbus RTU/TCP 工业设备监控与调试系统。它既可以作为项目展示时的操作脚本，也可以作为面试讲解提纲。

推荐演示方式：

```text
先用 Python 模拟器证明有可联调的设备
再用 smoke test 证明协议功能可自动验证
最后打开 Qt 上位机展示完整业务流程
```

## 1. 演示前准备

确认当前环境具备：

```text
Qt 6
CMake / Make
Python 3
pymodbus
```

项目路径：

```text
/Users/bbnn/Documents/QtProject/modbus
```

上位机源码路径：

```text
/Users/bbnn/Documents/QtProject/modbus/QtModbus
```

构建目录：

```text
/Users/bbnn/Documents/QtProject/modbus/build-vscode-intellisense
```

## 2. 构建项目

执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus
make -C build-vscode-intellisense
```

期望输出：

```text
[100%] Built target Modbus
```

讲解要点：

```text
项目使用 CMake 管理 Qt 工程。
构建目标是 Modbus。
工程链接了 Qt Widgets、Qt SerialBus、Qt SerialPort 和 Qt SQL。
```

## 3. 启动 Python Modbus TCP 模拟器

执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus/QtModbus
python3 simulator/modbus_tcp_server.py
```

模拟器默认监听：

```text
127.0.0.1:5020
```

模拟器提供的数据区：

```text
01 Coils            线圈
02 Discrete Inputs  离散输入
03 Holding Registers 保持寄存器
04 Input Registers 输入寄存器
```

模拟器数据含义：

```text
保持寄存器 0: 温度原始值，实际温度 = raw / 10.0
保持寄存器 1: 电压原始值，实际电压 = raw / 100.0
保持寄存器 2: 电流原始值，实际电流 = raw / 100.0
保持寄存器 3: 转速原始值，单位 rpm

输入寄存器 0-3: 与保持寄存器类似，用于测试功能码 04
线圈 0-3: 可写测试线圈
线圈 4-7: 动态变化测试线圈
离散输入 0-3: 动态状态和报警状态
```

如果出现端口占用：

```text
address already in use
```

说明 `5020` 端口已经有服务在运行，可以直接使用现有服务，或者关闭旧模拟器后重新启动。

## 4. 运行 smoke test

另开一个终端执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus/QtModbus
python3 simulator/modbus_smoke_test.py
```

期望输出：

```text
All Modbus smoke tests passed.
```

smoke test 覆盖：

```text
03 读取保持寄存器
04 读取输入寄存器
01 读取线圈
02 读取离散输入
06 写单个保持寄存器
16 写多个保持寄存器
05 写单个线圈
15 写多个线圈
```

讲解要点：

```text
这个脚本用于证明模拟器和 Modbus 功能码本身可用。
它也可以作为以后修改通信模块后的回归测试。
如果上位机 UI 出现问题，可以先用 smoke test 判断是通信层问题还是界面层问题。
```

## 5. 启动 Qt 上位机

执行：

```bash
cd /Users/bbnn/Documents/QtProject/modbus
./build-vscode-intellisense/Modbus
```

如果命令行启动受图形环境限制，也可以从 Qt Creator 或 IDE 中运行。

启动后应看到主窗口，包含以下功能页：

```text
设备连接
状态总览
寄存器调试
实时监控
实时曲线
采集配置
重连配置
报警记录
报警配置
历史查询
数据库维护
报文日志
```

## 6. 连接模拟设备

进入“设备连接”页面，填写：

```text
模式: Modbus TCP
IP: 127.0.0.1
端口: 5020
从站地址: 1
超时: 1000 ms
重试次数: 3
```

点击连接。

观察点：

```text
状态栏显示已连接
状态总览显示通信状态
报文日志出现 Connect request
连接成功后周期采集自动开始
```

讲解要点：

```text
连接操作由 ConnectionPanel 发出 connectRequested 信号。
MainWindow 将该信号连接到 QtModbusClient::connectDevice。
UI 层不直接操作 QModbusTcpClient。
```

## 7. 演示寄存器读取

进入“寄存器调试”页面。

### 读取保持寄存器 03

输入：

```text
数据区: 保持寄存器 03
起始地址: 0
读取数量: 4
```

预期：

```text
表格显示 4 行数据
地址 0-3 分别对应温度、电压、电流、转速
报文日志出现 Read holding registers
```

### 读取输入寄存器 04

输入：

```text
数据区: 输入寄存器 04
起始地址: 0
读取数量: 4
```

预期：

```text
表格显示 4 行数据
报文日志出现 Read input registers
```

### 读取线圈 01

输入：

```text
数据区: 线圈 01
起始地址: 0
读取数量: 8
```

预期：

```text
表格显示 0 或 1
地址 0-3 是可写测试线圈
地址 4-7 是动态测试线圈
```

### 读取离散输入 02

输入：

```text
数据区: 离散输入 02
起始地址: 0
读取数量: 4
```

预期：

```text
表格显示 0 或 1
离散输入用于表示只读状态量
```

讲解要点：

```text
功能码 01/02 面向位数据。
功能码 03/04 面向 16 位寄存器数据。
保持寄存器可读写，输入寄存器通常只读。
线圈可读写，离散输入通常只读。
```

## 8. 演示写入功能

### 写单个保持寄存器 06

输入：

```text
写入地址: 20
写入值: 1234
```

点击“写单个保持寄存器”。

验证：

```text
读取保持寄存器，起始地址 20，数量 1
表格应显示 1234
报文日志出现 write ok
```

### 写多个保持寄存器 16

输入：

```text
批量寄存器起始地址: 21
批量寄存器值: 100,200,300
```

点击“写多个保持寄存器”。

验证：

```text
读取保持寄存器，起始地址 21，数量 3
表格应显示 100、200、300
```

### 写单个线圈 05

输入：

```text
线圈地址: 0
线圈值: ON
```

点击“写单个线圈”。

验证：

```text
读取线圈，起始地址 0，数量 1
表格应显示 1
```

### 写多个线圈 15

输入：

```text
批量线圈起始地址: 0
批量线圈值: 1,0,1,1
```

点击“写多个线圈”。

验证：

```text
读取线圈，起始地址 0，数量 4
表格应显示 1、0、1、1
```

讲解要点：

```text
写入成功后 QtModbusClient 发出 registerWritten 信号。
MainWindow 将写入结果记录到报文日志，并更新状态栏。
批量写入结果使用 count 表示写入数量。
```

## 9. 演示周期采集

连接成功后，系统会自动启动周期轮询。

进入“实时监控”页面，观察：

```text
温度当前值
电压当前值
电流当前值
转速当前值
最小值
最大值
平均值
```

进入“实时曲线”页面，观察：

```text
温度曲线
电压曲线
电流曲线
转速曲线
```

讲解要点：

```text
PollingWorker 定时发出 readRequested。
QtModbusClient 读取保持寄存器后返回 RegisterReadResult。
PollingWorker 将原始寄存器转换为 EngineeringValue。
EngineeringValue 同时发送给实时监控、曲线、报警和数据库模块。
```

## 10. 演示采集配置

进入“采集配置”页面。

可以调整：

```text
采集周期
起始地址
读取数量
```

建议演示：

```text
将采集周期从 1000 ms 调整为 2000 ms
观察数据刷新频率变化
重启程序后配置仍然保留
```

讲解要点：

```text
采集配置通过 QSettings 持久化。
当设备已连接时，修改配置会立即重启轮询。
```

## 11. 演示报警功能

进入“报警配置”页面。

可以临时设置：

```text
温度上限低于当前温度
电压下限高于当前电压
```

触发报警后观察：

```text
报警记录页面新增报警
状态栏显示报警消息
报文日志出现 ALARM 记录
报警历史可以查询到记录
```

演示报警确认：

```text
在报警记录页面选中报警
点击确认
报警确认状态更新
数据库中的 confirmed 字段更新
```

讲解要点：

```text
AlarmManager 负责报警判断。
报警产生后通过 alarmRaised 信号通知 UI 和数据库。
报警确认通过 alarmConfirmed 信号通知 DatabaseManager。
```

## 12. 演示历史查询

进入“历史查询”页面。

演示内容：

```text
采集数据日志
历史曲线
报警日志
报文日志
```

可以说明：

```text
采集数据按时间范围查询
报警日志支持确认状态筛选
报文日志支持分类筛选
查询结果支持 CSV 导出
```

讲解要点：

```text
DatabaseManager 统一管理 SQLite 连接。
采集数据、报警日志、报文日志分表存储。
查询 UI 不直接写 SQL，而是调用 DatabaseManager 的查询接口。
```

## 13. 演示数据库维护

进入“数据库维护”页面。

演示：

```text
查看 collect_data 行数
查看 alarm_log 行数
查看 packet_log 行数
清理指定时间之前的数据
执行 VACUUM
```

讲解要点：

```text
工业上位机长时间运行会产生大量历史数据。
数据库维护功能用于控制本地数据库体积。
清理旧数据后执行 VACUUM 可以回收 SQLite 文件空间。
```

## 14. 演示自动重连

进入“重连配置”页面。

设置：

```text
启用自动重连
重连间隔: 3000 ms
```

演示步骤：

```text
1. 确保上位机已连接模拟器。
2. 停止模拟器。
3. 观察状态栏显示断开或通信错误。
4. 观察报文日志出现自动重连消息。
5. 重新启动模拟器。
6. 观察连接恢复。
```

讲解要点：

```text
主动断开不会触发自动重连。
异常断开会触发 unexpectedDisconnected。
ReconnectController 保存最近一次连接配置，并按间隔发起重连。
```

## 15. 演示报文日志

进入“报文日志”页面。

依次执行：

```text
连接设备
读取保持寄存器
写单个寄存器
写多个寄存器
读取线圈
写线圈
触发报警
停止模拟器触发重连
```

观察日志类型：

```text
TX: 用户操作或请求
RX: 通信结果或错误
ALARM: 报警事件
AUTO: 自动重连事件
DB: 数据库错误
```

讲解要点：

```text
报文日志是工业通信调试的重要工具。
当前记录的是操作摘要和结果，后续可以扩展真实十六进制原始报文。
```

## 16. 常见问题处理

### 模拟器无法启动

现象：

```text
address already in use
```

原因：

```text
5020 端口已经被占用，可能已有一个模拟器正在运行。
```

处理：

```text
关闭旧模拟器，或直接使用已有模拟器进行测试。
```

### smoke test 无法连接

现象：

```text
Cannot connect to 127.0.0.1:5020
```

检查：

```text
模拟器是否已启动
端口是否为 5020
是否被系统权限或沙箱限制本地网络连接
```

### 上位机连接失败

检查：

```text
IP 是否为 127.0.0.1
端口是否为 5020
从站地址是否为 1
模拟器是否正在运行
防火墙或系统权限是否阻止连接
```

### 读写结果和预期不一致

检查：

```text
读取的数据区是否正确
起始地址是否正确
读取数量是否正确
写入后是否读取了对应地址
周期轮询是否正在刷新保持寄存器 0-3
```

## 17. 面试讲解顺序

推荐按下面顺序讲：

```text
1. 项目定位
   这是一个工业设备监控与调试系统，不是简单寄存器工具。

2. 通信封装
   使用 IModbusClient 抽象通信接口，QtModbusClient 负责具体 Qt SerialBus 实现。

3. 功能码支持
   支持 01/02/03/04/05/06/15/16，覆盖常见读取和写入场景。

4. 周期采集
   PollingWorker 定时读取保持寄存器，并转换成工程量。

5. 实时监控
   UI 展示当前值、统计值和实时趋势。

6. 报警处理
   AlarmManager 独立判断阈值和离线报警，报警可确认并持久化。

7. 数据存储
   SQLite 保存采集数据、报警日志和报文日志。

8. 自动重连
   异常断开时根据最近一次连接配置自动重连。

9. 联调验证
   Python 模拟器和 smoke test 证明协议功能可验证、可复现。
```

## 18. 一分钟版本讲解

如果时间很短，可以这样说：

```text
这个项目是一个基于 Qt 的 Modbus 工业上位机系统，支持 TCP/RTU 连接、常用功能码读写、周期采集、实时曲线、报警、SQLite 历史存储和自动重连。

架构上我把 UI、通信、采集、报警、数据库拆成独立模块，通过 Qt 信号槽解耦。UI 不直接操作 QModbusTcpClient，而是通过 IModbusClient 接口调用通信层。

为了方便联调，我还写了 Python Modbus TCP 模拟器和 smoke test，能自动验证 01/02/03/04/05/06/15/16 这些功能码，保证项目不是只停留在界面展示，而是可以实际通信验证。
```
