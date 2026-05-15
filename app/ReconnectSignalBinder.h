#ifndef RECONNECTSIGNALBINDER_H
#define RECONNECTSIGNALBINDER_H

#include "MainWindowSignalBinder.h"

// 绑定自动重连流程相关信号槽。
class ReconnectSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // RECONNECTSIGNALBINDER_H
