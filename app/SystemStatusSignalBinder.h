#ifndef SYSTEMSTATUSSIGNALBINDER_H
#define SYSTEMSTATUSSIGNALBINDER_H

#include "MainWindowSignalBinder.h"

// 绑定状态总览面板的连接、采集、报警状态更新信号。
class SystemStatusSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // SYSTEMSTATUSSIGNALBINDER_H
