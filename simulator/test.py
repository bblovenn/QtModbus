from pymodbus.client import ModbusTcpClient

# smoke test 覆盖上位机支持的常用 Modbus 功能码，适合作为改代码后的快速回归。
HOST = "127.0.0.1"
PORT = 5020
DEVICE_ID = 1

#验证响应
def require_ok(name, response):
    if response is None:
        raise RuntimeError(f"{name} failed: no response")

    if response.isError():
        raise RuntimeError(f"{name} failed: {response}")

    print(f"[OK] {name}: {response}")
    return response


def main():
    # 脚本连接已启动的本地模拟器；不自动启动服务，方便和 GUI 联调用同一个实例。
    client = ModbusTcpClient(HOST, port=PORT) #创建 Modbus TCP 客户端实例

    if not client.connect():
        raise RuntimeError(f"Cannot connect to {HOST}:{PORT}")

    try:
        # 03 功能码：读保持寄存器
        hr = require_ok(
            "03 read holding registers",
            client.read_holding_registers(address=0, count=4, device_id=DEVICE_ID),
        )
        print("    HR:", hr.registers)

        # 04 功能码：读输入寄存器
        ir = require_ok(
            "04 read input registers",
            client.read_input_registers(address=0, count=4, device_id=DEVICE_ID),
        )
        print("    IR:", ir.registers)

        # 01 功能码：读线圈
        coils = require_ok(
            "01 read coils",
            client.read_coils(address=0, count=8, device_id=DEVICE_ID),
        )
        print("    Coils:", coils.bits[:8])

        # 02 功能码：读离散输入
        discrete_inputs = require_ok(
            "02 read discrete inputs",
            client.read_discrete_inputs(address=0, count=4, device_id=DEVICE_ID),
        )
        print("    DI:", discrete_inputs.bits[:4])

        # 06 功能码：写单个保持寄存器
        require_ok(
            "06 write single holding register",
            client.write_register(address=20, value=1234, device_id=DEVICE_ID),
        )

        verify_single_hr = require_ok(
            "verify single holding register",
            client.read_holding_registers(address=20, count=1, device_id=DEVICE_ID),
        )
        assert verify_single_hr.registers[0] == 1234

        # 16 功能码：写多个保持寄存器
        require_ok(
            "16 write multiple holding registers",
            client.write_registers(address=21, values=[100, 200, 300], device_id=DEVICE_ID),
        )

        # 验证写入的多个保持寄存器
        verify_multi_hr = require_ok(
            "verify multiple holding registers",
            client.read_holding_registers(address=21, count=3, device_id=DEVICE_ID),
        )
        assert verify_multi_hr.registers == [100, 200, 300]

        # 05 功能码：写单个线圈
        require_ok(
            "05 write single coil",
            client.write_coil(address=0, value=True, device_id=DEVICE_ID),
        )

        verify_single_coil = require_ok(
            "verify single coil",
            client.read_coils(address=0, count=1, device_id=DEVICE_ID),
        )
        assert verify_single_coil.bits[0] is True

        # 15 功能码：写多个线圈
        require_ok(
            "15 write multiple coils",
            client.write_coils(address=0, values=[True, False, True, True], device_id=DEVICE_ID),
        )

        verify_multi_coils = require_ok(
            "verify multiple coils",
            client.read_coils(address=0, count=4, device_id=DEVICE_ID),
        )
        assert verify_multi_coils.bits[:4] == [True, False, True, True]

        print("\nAll tests passed.")

    finally:
        client.close()


if __name__ == "__main__":
    main()
