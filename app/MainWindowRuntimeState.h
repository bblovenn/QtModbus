#ifndef MAINWINDOWRUNTIMESTATE_H
#define MAINWINDOWRUNTIMESTATE_H

// 主窗口运行态：只保存运行期间会被信号槽共同读写的轻量状态。
struct MainWindowRuntimeState
{
    int pollingIntervalMs = 1000;
    int pollingStartAddress = 0;
    int pollingCount = 4;
    bool modbusConnected = false;
};

#endif // MAINWINDOWRUNTIMESTATE_H
