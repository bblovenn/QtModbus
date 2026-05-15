import random
import time

# 本脚本是本地 Modbus TCP 模拟器，用于没有真实下位机时验证上位机读写流程。
from pymodbus.server import StartTcpServer
from pymodbus.simulator import DataType
from pymodbus.simulator import SimData
from pymodbus.simulator import SimDevice

sensor_values = [250, 2400, 500, 1200]
bit_tick = 0


def update_sensor_values():
    # 模拟传感器缓慢抖动，并限制在合理范围内，避免趋势曲线跳变过大。
    sensor_values[0] += random.randint(-2, 2)
    sensor_values[1] += random.randint(-5, 5)
    sensor_values[2] += random.randint(-10, 10)
    sensor_values[3] += random.randint(-20, 20)

    sensor_values[0] = max(0, min(1000, sensor_values[0]))
    sensor_values[1] = max(0, min(3000, sensor_values[1]))
    sensor_values[2] = max(0, min(2000, sensor_values[2]))
    sensor_values[3] = max(0, min(3000, sensor_values[3]))


def sensor_alarm_bits():
    global bit_tick

    temperature, voltage, current, speed = sensor_values
    bit_tick += 1

    return [
        bool(bit_tick % 2),
        bool((bit_tick // 2) % 2),
        temperature >= 250,
        speed >= 1200,
    ]


def bits_to_mask(bits):
    value = 0

    for index, bit in enumerate(bits):
        if bit:
            value |= 1 << index

    return value


def dynamic_coil_bits():
    seconds = int(time.time())

    return [
        bool(seconds % 2),
        bool((seconds // 2) % 2),
        bool((seconds // 4) % 2),
        bool((seconds // 8) % 2),
    ]


async def update_data(
    function_code,
    start_address,
    address,
    count,
    current_registers,
    set_values,
):
    # pymodbus simulator 会在请求到来时回调这里，用功能码决定更新哪一类数据区。
    del start_address, address, count, set_values

    # 03: Holding Registers, 04: Input Registers
    if function_code in (3, 4):
        update_sensor_values()
        current_registers[0:4] = sensor_values
        return None

    # 02: Discrete Inputs
    # SimDevice 的位区底层按 16 位寄存器打包，写入 bitmask 才能稳定反映到 02 读结果。
    if function_code == 2:
        current_registers[0] = bits_to_mask(sensor_alarm_bits())
        return None

    # 01: Coils
    # 低 4 位保留给上位机写入测试，高 4 位做动态演示，读地址 4-7 可看到变化。
    if function_code == 1:
        writable_low_bits = current_registers[0] & 0x000F
        dynamic_high_bits = bits_to_mask(dynamic_coil_bits()) << 4
        current_registers[0] = writable_low_bits | dynamic_high_bits
        return None

    return None


device = SimDevice(
    id=0,
    simdata=(
        [SimData(0, count=100, values=False, datatype=DataType.BITS)],      # 01 Coils
        [SimData(0, count=100, values=False, datatype=DataType.BITS)],      # 02 Discrete Inputs
        [SimData(0, count=100, values=0, datatype=DataType.REGISTERS)],     # 03 Holding Registers
        [SimData(0, count=100, values=0, datatype=DataType.REGISTERS)],     # 04 Input Registers
    ),
    action=update_data,
)

if __name__ == "__main__":
    print("Modbus TCP simulator running at 127.0.0.1:5020")
    print("Registers:")
    print("  HR/IR 0: temperature x10")
    print("  HR/IR 1: voltage x100")
    print("  HR/IR 2: current x100")
    print("  HR/IR 3: speed rpm")
    print("  HR 10-19: writable test registers, values persist until overwritten")
    print("Discrete inputs:")
    print("  DI 0-1: dynamic test bits")
    print("  DI 2: temperature >= 25.0")
    print("  DI 3: speed >= 1200")
    print("Coils:")
    print("  C 0-3: writable by Modbus function 05/15")
    print("  C 4-7: dynamic test bits")

    StartTcpServer(context=device, address=("127.0.0.1", 5020))
