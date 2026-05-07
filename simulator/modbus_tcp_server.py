import random
import time
from threading import Thread

from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusServerContext
from pymodbus.datastore import ModbusSlaveContext
from pymodbus.server import StartTcpServer

store = ModbusSlaveContext(
    hr=ModbusSequentialDataBlock(0, [0] * 100), #保持寄存器
    ir=ModbusSequentialDataBlock(0, [0] * 100), #输入寄存器
)

# 单从机模式，所有请求都被路由到同一个从机上下文
context = ModbusServerContext(slaves=store, single=True)

def update_data():
    # 初始值（原始寄存器值，需除以倍率才是真实工程值）
    temperature = 250
    voltage = 2400
    current = 500
    speed = 1200

    while True:
        temperature += random.randint(-2, 2)
        voltage += random.randint(-5, 5)
        current += random.randint(-10, 10)
        speed += random.randint(-20, 20)

        # 限幅保护
        temperature = max(0, min(1000, temperature))
        voltage = max(0, min(3000, voltage))
        current = max(0, min(2000, current))
        speed = max(0, min(3000, speed))

        values = [
            temperature,
            voltage,
            current,
            speed,
        ]
        # 功能码 3：保持寄存器（Holding Registers）
        context[0].setValues(3, 0, values)
        # 功能码 4：输入寄存器（Input Registers）
        context[0].setValues(4, 0, values)

        time.sleep(1)


if __name__ == "__main__":
    # 启动数据更新线程（守护线程，主进程退出时自动终止）
    Thread(target=update_data, daemon=True).start()

    print("Modbus TCP simulator running at 127.0.0.1:5020")
    print("Holding registers:")
    print("  0: temperature x10")
    print("  1: voltage x100")
    print("  2: current x100")
    print("  3: speed rpm")

    StartTcpServer(context=context, address=("127.0.0.1", 5020))
