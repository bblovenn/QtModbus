#ifndef PACKETLOGSIGNALBINDER_H
#define PACKETLOGSIGNALBINDER_H

#include "MainWindowSignalBinder.h"

// 绑定所有会生成报文日志的信号，包括连接、读写、错误和报警事件。
class PacketLogSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // PACKETLOGSIGNALBINDER_H
