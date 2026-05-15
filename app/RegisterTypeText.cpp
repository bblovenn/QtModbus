#include "RegisterTypeText.h"

QString registerTypeText(RegisterType type)
{
    switch (type) {
    case RegisterType::HoldingRegister:
        return "保持寄存器";
    case RegisterType::InputRegister:
        return "输入寄存器";
    case RegisterType::Coil:
        return "线圈";
    case RegisterType::DiscreteInput:
        return "离散输入";
    default:
        return "未知类型";
    }
}
