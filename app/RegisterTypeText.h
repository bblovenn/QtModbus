#ifndef REGISTERTYPETEXT_H
#define REGISTERTYPETEXT_H

#include "../core/RegisterValue.h"

#include <QString>

// 将内部寄存器类型转换为面向用户的中文文本。
QString registerTypeText(RegisterType type);

#endif // REGISTERTYPETEXT_H
