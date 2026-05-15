#ifndef POLLINGALARMSIGNALBINDER_H
#define POLLINGALARMSIGNALBINDER_H

#include "MainWindowSignalBinder.h"

// 绑定轮询采集、实时监控和报警判断相关信号槽。
class PollingAlarmSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // POLLINGALARMSIGNALBINDER_H
