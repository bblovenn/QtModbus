# QtModbus Structure Refactor Design

## Goal

整理项目结构，降低 `MainWindow` 和构建文件的维护成本，同时保持现有功能、UI 布局、数据库文件、模拟器和 smoke test 行为不变。

## Scope

本次采用中等重构：

- 保留现有目录：`core/`、`communication/`、`polling/`、`alarm/`、`database/`、`ui/`、`simulator/`。
- 新增轻量应用层目录 `app/`，承载主窗口编排辅助类型。
- 抽出应用设置读写，避免 `MainWindow` 中散落多个 `QSettings` 读写片段。
- 抽出主窗口面板集合和日志辅助结构，让 `MainWindow` 更像页面装配入口，而不是所有对象和信号的混合脚本。
- 整理 CMake 源码列表，避免 Android 和桌面目标重复维护同一批文件。

## Architecture

### `app/AppSettings`

负责集中读写持久化配置：

- 连接配置：模式、TCP 地址、串口、从站 ID。
- 轮询配置：周期、起始地址、数量。
- 报警阈值：温度上限、电压下限。
- 重连配置：启用状态、重连间隔。

它只封装 `QSettings` 键名和默认值，不持有 UI、通信客户端或数据库对象。

### `app/MainWindowPanels`

提供一个轻量 struct，集中保存 `MainWindow` 创建的主要面板指针：

- 连接、寄存器、监控、趋势、报警、历史、数据库维护、报文日志等面板。
- 主 tab 和历史子 tab。

它不拥有复杂逻辑；对象生命周期仍由 Qt parent 管理。

### `MainWindow`

调整为几个清晰阶段：

- 创建业务对象。
- 创建面板。
- 从 `AppSettings` 加载初始配置并填充面板。
- 添加 tab。
- 打开数据库。
- 连接信号槽。

现有信号槽行为保持一致，只把代码按职责拆小。

### Build Files

`CMakeLists.txt` 使用统一源码变量，例如 `MODBUS_SOURCES`。Android 使用 `add_library`，桌面使用 `add_executable`，两者引用同一份源码清单。

`Modbus.pro` 暂时只同步新增 `app/` 文件，不重写 qmake 结构。

## Behavior Preservation

以下行为必须保持不变：

- 构建目标仍为 `Modbus`。
- 主窗口标题、tab 顺序、默认连接参数、默认轮询参数不变。
- `modbus_hmi.db` 数据库文件名不变。
- 报警、采集、报文日志、自动重连信号链路不变。
- Python 模拟器和 `simulator/test.py` 不变。

## Verification

完成后至少运行：

```bash
make -C /Users/bbnn/Documents/QtProject/modbus/build-vscode-intellisense Modbus
```

如果本地模拟器运行中，再运行：

```bash
python3 simulator/test.py
```

若模拟器未运行，记录 smoke test 未执行原因。
