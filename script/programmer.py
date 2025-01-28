import serial
import struct
import time
import hashlib
from typing import List
from intelhex import IntelHex  # 用于解析 Intel Hex 文件
import os

# STM32F103的扇区大小（4KB）
SECTOR_SIZE = 0x1000  # 4KB

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

# 擦除flash区域的函数
def erase_flash(serial_comm: SerialCommunication, start_address: int, size: int):
    erase_size = max(size, SECTOR_SIZE)
    # 擦除命令：起始地址 + 擦除大小
    data = struct.pack("<I", start_address) + struct.pack("<I", erase_size)
    frame = generate_frame(0x24, len(data), list(data))
    serial_comm.send_frame(frame)
    print(f"擦除地址 {start_address:#010x}, 大小 {erase_size:#010x}")

    # 等待擦除响应
    response = serial_comm.receive_frame()
    if response and response[6] == 0x44 and response[9] == 0x00:
        print(f"擦除成功")
    else:
        print(f"擦除失败")
        return False
    return True

# 写入flash数据的函数
def write_flash(serial_comm: SerialCommunication, start_address: int, size: int, data: bytes):
    offset = 0
    write_addr = start_address + offset
    while offset < size:
        if size - offset >= SECTOR_SIZE:
            chunk = data[offset:offset + SECTOR_SIZE]
        else:
            chunk = data[offset:offset + (size - offset)]
        # 将数据打包并发送
        data_to_send = struct.pack("<I", write_addr) + struct.pack("<H", len(chunk)) + bytes(chunk)
        frame = generate_frame(0x25, len(data_to_send), list(data_to_send))
        serial_comm.send_frame(frame)
        print(f"写入地址 {write_addr:#010x}, 数据大小 {len(chunk)}")

        # 等待写入响应
        response = serial_comm.receive_frame()
        if response and response[6] == 0x45 and response[9] == 0x00:
            print(f"写入成功，地址 {write_addr:#010x}")
            # 更新偏移量和写入地址
            offset += len(chunk)
            write_addr += len(chunk)
        else:
            print(f"写入失败，地址 {write_addr:#010x}")
            return False
    return True

# 复位芯片的函数
def reset_chip(serial_comm: SerialCommunication):
    # 复位命令，无数据
    frame = generate_frame(0x27, 0, [])
    serial_comm.send_frame(frame)
    print("发送复位芯片命令")

    # 等待复位响应
    response = serial_comm.receive_frame()
    if response and response[6] == 0x47:
        if response[9] == 0x00:
            print("复位芯片成功")
            return True
        else:
            print("复位芯片失败")
            return False
    print("复位芯片响应失败")
    return False

# 主函数
def main():
    # 设置串口连接
    port = "COM4"  # 这里根据实际的串口端口进行修改
    baudrate = 115200
    serial_comm = SerialCommunication(port, baudrate)

    # 设置默认 passcode (HEX: 01 02 03 04 05 06)
    passcode = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06])

    # 设置烧写的 hex 文件
    hex_file = "firmware.hex"  # 请根据实际路径调整

    # 读取 Intel Hex 文件
    hex_data = IntelHex(hex_file)

    # 获取所有的数据段（段的起始地址和结束地址）
    segments = list(hex_data.segments())

    # 打印出所有的段及其大小
    if segments:
        print("Hex 文件包含数据段：")
        for start, stop in segments:
            size = stop - start
            print(f"地址段: {start:#010x} - {stop:#010x}, 数据大小: {size} 字节")
    else:
        print("Hex 文件不包含任何数据段。")

    try:
        # 请求连接
        if request_connection(serial_comm):
            # 握手1
            handshake_data1 = handshake_1(serial_comm)
            if handshake_data1:
                # 握手2
                if handshake_2(serial_comm, handshake_data1, passcode):
                    for start, stop in segments:
                        size = stop - start
                        # 擦除指定区域
                        if erase_flash(serial_comm, start, size):
                            # 写入 hex 文件数据
                            if write_flash(serial_comm, start, size, hex_data.tobinarray(start, stop)):
                                 # 写入成功后复位芯片
                                if reset_chip(serial_comm):
                                    print("固件编程成功，芯片已复位")
                                else:
                                    print("固件编程成功，但复位芯片失败")
    finally:
        # 关闭串口
        serial_comm.close()

if __name__ == "__main__":
    main()
