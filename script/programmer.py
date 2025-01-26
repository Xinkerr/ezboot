import serial
import struct
import time
import hashlib
from typing import List

# CRC16计算函数（从提供的pg_tool.py中提取）
def reverse_bits(value: int, bit_width: int = 16) -> int:
    """反转指定宽度的位"""
    reversed_value = 0
    for _ in range(bit_width):
        reversed_value = (reversed_value << 1) | (value & 1)
        value >>= 1
    return reversed_value

def crc16_ccitt(data: bytes, seed: int = 0x0000) -> int:
    """计算 CRC16 CCITT 校验码"""
    for byte in data:
        byte = reverse_bits(byte, 8)  # 输入数据反转
        seed ^= byte << 8
        for _ in range(8):
            if seed & 0x8000:
                seed = (seed << 1) ^ 0x1021
            else:
                seed <<= 1
            seed &= 0xFFFF
    return reverse_bits(seed, 16)  # 输出数据反转

# 构造数据帧
def generate_frame(tag: int, value_length: int, value: List[int]) -> bytes:
    tlv_data = struct.pack("<B", tag) + struct.pack("<H", value_length) + bytes(value)
    head = 0x55AA
    payload_length = len(tlv_data)
    length_check = crc16_ccitt(struct.pack("<H", payload_length))
    
    frame_without_crc = (
        struct.pack("<H", head) +
        struct.pack("<H", payload_length) +
        struct.pack("<H", length_check) +
        tlv_data
    )
    
    crc = crc16_ccitt(frame_without_crc)
    frame = frame_without_crc + struct.pack("<H", crc)
    return frame

# 串口通信类
class SerialCommunication:
    def __init__(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate, timeout=2)  # 增加超时时间

    def send_frame(self, frame: bytes):
        self.ser.write(frame)
        print(f"发送数据帧: {' '.join(f'{byte:02X}' for byte in frame)}")
        
    def receive_frame(self):
        timeout_counter = 0
        while self.ser.in_waiting == 0 and timeout_counter < 5:
            print("等待数据...")
            time.sleep(0.5)  # 等待 0.5 秒再检查一次
            timeout_counter += 1
        
        if self.ser.in_waiting > 0:
            response = self.ser.read(self.ser.in_waiting)
            print(f"接收到数据帧: {' '.join(f'{byte:02X}' for byte in response)}")
            return response
        else:
            print("接收超时，没有数据。")
            return None
    
    def close(self):
        self.ser.close()

# 请求连接的函数
def request_connection(serial_comm: SerialCommunication):
    # 请求建立连接，发送数据帧
    data = [0x01]  # 0x01 请求建立连接
    frame = generate_frame(0x21, len(data), data)
    print("请求连接数据帧:")
    print(f"发送数据: {' '.join(f'{byte:02X}' for byte in frame)}")
    serial_comm.send_frame(frame)

    # 等待响应
    response = serial_comm.receive_frame()
    if response:
        print(f"接收到数据: {' '.join(f'{byte:02X}' for byte in response)}")
        # 解析响应是否为成功应答
        if response[6] == 0x41:
            print("连接请求成功，开始握手1。")
            return True
        else:
            print("连接请求失败，响应数据不符合预期。")
            if response[6] == 0xFF:
                print("设备拒绝连接请求 (0xFF)。")
            else:
                print(f"未知的响应: {response[6]}")
    else:
        print("未收到设备响应。")
    return False

# 握手1的函数
def handshake_1(serial_comm: SerialCommunication):
    # 握手1请求，发送数据帧
    data = []  # 无数据，直接请求
    frame = generate_frame(0x22, len(data), data)
    serial_comm.send_frame(frame)

    # 等待响应
    response = serial_comm.receive_frame()
    if response and response[6] == 0x42:
        handshake_data1 = response[9:13]  # 修改为从 index 9 到 12 提取握手数据1
        print(f"握手1成功，握手数据1: {handshake_data1.hex()}")
        return handshake_data1
    print("握手1失败。")
    return None

# 计算握手数据2：SHA256(握手数据1 + passcode)
def calculate_handshake2(handshake_data1: bytes, passcode: bytes) -> bytes:
    # 拼接握手数据1和passcode
    data_to_hash = handshake_data1 + passcode
    # 使用 SHA-256 计算哈希
    sha256_hash = hashlib.sha256(data_to_hash).digest()
    return sha256_hash

# 握手2的函数
def handshake_2(serial_comm: SerialCommunication, handshake_data1: bytes, passcode: bytes):
    # 计算握手数据2：SHA256(握手数据1 + passcode)
    handshake_data2 = calculate_handshake2(handshake_data1, passcode)
    print(f"握手数据2: {handshake_data2.hex()}")

    # 握手2请求，发送数据帧
    frame = generate_frame(0x23, len(handshake_data2), list(handshake_data2))
    serial_comm.send_frame(frame)

    # 等待响应
    response = serial_comm.receive_frame()
    if response and response[6] == 0x43:
        if response[9] == 0x00:
            print("握手2成功。")
            return True
        else:
            print("握手2失败。")
            return False
    print("握手2响应失败。")
    return False

# 主函数
def main():
    # 设置串口连接
    port = "COM3"  # 这里根据实际的串口端口进行修改
    baudrate = 115200
    serial_comm = SerialCommunication(port, baudrate)

    # 设置默认 passcode (HEX: 01 02 03 04 05 06)
    passcode = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06])

    try:
        # 请求连接
        if request_connection(serial_comm):
            # 握手1
            handshake_data1 = handshake_1(serial_comm)
            if handshake_data1:
                # 握手2
                handshake_2(serial_comm, handshake_data1, passcode)
    finally:
        # 关闭串口
        serial_comm.close()

if __name__ == "__main__":
    main()
