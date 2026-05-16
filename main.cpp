// Modbus RTU/TCP 工业设备监控与调试系统 — 应用入口
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // 进入 Qt 事件循环，所有 Modbus 通信、轮询、报警都由信号槽驱动
    return a.exec();
}
