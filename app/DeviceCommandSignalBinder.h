#ifndef DEVICECOMMANDSIGNALBINDER_H
#define DEVICECOMMANDSIGNALBINDER_H

#include "MainWindowSignalBinder.h"

// 绑定设备连接和寄存器读写命令相关信号槽。
class DeviceCommandSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // DEVICECOMMANDSIGNALBINDER_H
