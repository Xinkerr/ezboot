import sys
import serial
import struct
import time
import hashlib
import argparse  # 导入 argparse 模块
from typing import List, Optional, Tuple
from intelhex import IntelHex  # 用于解析 Intel Hex 文件
from tqdm import tqdm  # 导入 tqdm 库用于进度条

# STM32F103的扇区大小（4KB）
SECTOR_SIZE = 0x1000  # 4KB

# 最大重试次数
MAX_RETRIES = 3

# 增加调试开关，默认关闭
DEBUG = False

def debug_print(*args, **kwargs):
    """根据 DEBUG 变量的状态决定是否打印调试信息"""
    if DEBUG:
        print(*args, **kwargs)

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
        self.ser = serial.Serial(port, baudrate, timeout=1)

    def send_frame(self, frame: bytes):
        self.ser.write(frame)
        debug_print(f"发送数据帧: {' '.join(f'{byte:02X}' for byte in frame)}")

    def receive_frame(self) -> Optional[bytes]:
        timeout_counter = 0
        while self.ser.in_waiting == 0 and timeout_counter < 10:
            debug_print("等待数据...")
            time.sleep(0.2)  # 等待 0.5 秒再检查一次
            timeout_counter += 1
        
        if self.ser.in_waiting > 0:
            response = self.ser.read(self.ser.in_waiting)
            debug_print(f"接收到数据帧: {' '.join(f'{byte:02X}' for byte in response)}")
            return response
        else:
            debug_print("接收超时，没有数据。")
            return None
    
    def close(self):
        self.ser.close()

# 检查接收到的数据帧是否符合协议格式
def validate_frame(frame: bytes) -> bool:
    if len(frame) < 8:  # 最小帧长度：HEAD(2) + PAYLOAD LENGTH(2) + LENGTH CHECK(2) + CRC(2)
        debug_print("数据帧长度不足")
        return False

    # 检查协议头
    head = struct.unpack("<H", frame[0:2])[0]
    if head != 0x55AA:
        debug_print(f"协议头错误，期望 0x55AA，实际 {head:#06X}")
        return False

    # 检查 PAYLOAD LENGTH
    payload_length = struct.unpack("<H", frame[2:4])[0]
    if len(frame) != 8 + payload_length:  # HEAD(2) + PAYLOAD LENGTH(2) + LENGTH CHECK(2) + PAYLOAD(n) + CRC(2)
        debug_print(f"PAYLOAD 长度不匹配，期望 {payload_length}，实际 {len(frame) - 8}")
        return False

    # 检查 LENGTH CHECK
    length_check = struct.unpack("<H", frame[4:6])[0]
    expected_length_check = crc16_ccitt(struct.pack("<H", payload_length))
    if length_check != expected_length_check:
        debug_print(f"LENGTH CHECK 错误，期望 {expected_length_check:#06X}，实际 {length_check:#06X}")
        return False

    # 检查 CRC
    received_crc = struct.unpack("<H", frame[-2:])[0]
    expected_crc = crc16_ccitt(frame[:-2])
    if received_crc != expected_crc:
        debug_print(f"CRC 校验失败，期望 {expected_crc:#06X}，实际 {received_crc:#06X}")
        return False

    debug_print("数据帧校验成功")
    return True

# 发送数据帧并等待响应，支持重试机制
def send_and_receive(serial_comm: SerialCommunication, frame: bytes, expected_tag: int) -> Tuple[bool, Optional[bytes]]:
    for retry in range(MAX_RETRIES):
        serial_comm.send_frame(frame)
        response = serial_comm.receive_frame()
        if response and validate_frame(response):
            if response[6] == expected_tag:
                return True, response
            else:
                debug_print(f"响应 TAG 不匹配，期望 {expected_tag:#04X}，实际 {response[6]:#04X}")
        else:
            debug_print(f"第 {retry + 1} 次重试...")
            time.sleep(1)  # 等待 1 秒后重试
    return False, None

# 请求连接的函数
def request_connection(serial_comm: SerialCommunication) -> bool:
    while True:
        # 请求建立连接，发送数据帧
        data = [0x01]  # 0x01 请求建立连接
        frame = generate_frame(0x21, len(data), data)
        debug_print(f"请求连接数据: {' '.join(f'{byte:02X}' for byte in frame)}")

        success, response = send_and_receive(serial_comm, frame, 0x41)
        if success:
            debug_print("连接成功")
            return True

# 握手1的函数
def handshake_1(serial_comm: SerialCommunication) -> Optional[bytes]:
    # 握手1请求，发送数据帧
    data = []  # 无数据，直接请求
    frame = generate_frame(0x22, len(data), data)

    success, response = send_and_receive(serial_comm, frame, 0x42)
    if success:
        handshake_data1 = response[9:13]  # 修改为从 index 9 到 12 提取握手数据1
        debug_print("握手1成功")
        debug_print(f"握手数据1: {handshake_data1.hex()}")
        return handshake_data1
    else:
        debug_print("握手1失败。")
        return None

# 计算握手数据2：SHA256(握手数据1 + passcode)
def calculate_handshake2(handshake_data1: bytes, passcode: bytes) -> bytes:
    # 拼接握手数据1和passcode
    data_to_hash = handshake_data1 + passcode
    # 使用 SHA-256 计算哈希
    sha256_hash = hashlib.sha256(data_to_hash).digest()
    return sha256_hash

# 握手2的函数
def handshake_2(serial_comm: SerialCommunication, handshake_data1: bytes, passcode: bytes) -> bool:
    # 计算握手数据2：SHA256(握手数据1 + passcode)
    handshake_data2 = calculate_handshake2(handshake_data1, passcode)
    debug_print(f"握手数据2: {handshake_data2.hex()}")

    # 握手2请求，发送数据帧
    frame = generate_frame(0x23, len(handshake_data2), list(handshake_data2))

    success, response = send_and_receive(serial_comm, frame, 0x43)
    if success:
        if response[9] == 0x00:
            debug_print("握手2成功")
            return True
        else:
            debug_print("握手2失败。")
            return False
    else:
        debug_print("握手2响应失败。")
        return False

# 擦除flash区域的函数
def erase_flash(serial_comm: SerialCommunication, start_address: int, size: int) -> bool:
    erase_size = max(size, SECTOR_SIZE)
    # 擦除命令：起始地址 + 擦除大小
    data = struct.pack("<I", start_address) + struct.pack("<I", erase_size)
    frame = generate_frame(0x24, len(data), list(data))

    success, response = send_and_receive(serial_comm, frame, 0x44)
    if success and response[9] == 0x00:
        debug_print(f"擦除成功")
        return True
    else:
        debug_print(f"擦除失败")
        return False

# 写入flash数据的函数
def write_flash(serial_comm: SerialCommunication, start_address: int, size: int, data: bytes) -> bool:
    offset = 0
    write_addr = start_address + offset
    total_size = size  # 总数据大小
    with tqdm(total=total_size, unit='B', unit_scale=True, desc="写入进度") as pbar:  # 使用 tqdm 创建进度条
        while offset < size:
            if size - offset >= SECTOR_SIZE:
                chunk = data[offset:offset + SECTOR_SIZE]
            else:
                chunk = data[offset:offset + (size - offset)]
            # 将数据打包并发送
            data_to_send = struct.pack("<I", write_addr) + struct.pack("<H", len(chunk)) + bytes(chunk)
            frame = generate_frame(0x25, len(data_to_send), list(data_to_send))

            success, response = send_and_receive(serial_comm, frame, 0x45)
            if success and response[9] == 0x00:
                debug_print(f"写入成功，地址 {write_addr:#010x}")
                # 更新偏移量和写入地址
                offset += len(chunk)
                write_addr += len(chunk)
                pbar.update(len(chunk))  # 更新进度条
            else:
                debug_print(f"写入失败，地址 {write_addr:#010x}")
                return False
    return True

# 复位芯片的函数
def reset_chip(serial_comm: SerialCommunication) -> bool:
    # 复位命令，无数据
    frame = generate_frame(0x27, 0, [])

    success, response = send_and_receive(serial_comm, frame, 0x47)
    if success and response[9] == 0x00:
        debug_print("复位芯片成功")
        return True
    else:
        debug_print("复位芯片失败")
        return False

def process_flash_operations(serial_comm, segments, passcode, hex_data):
    print("请求连接")
    if request_connection(serial_comm):
        print("连接成功")
    else:
        print("连接失败")
        return False

    handshake_data1 = handshake_1(serial_comm)
    if handshake_data1:
        print("握手1成功")
    else:
        print("握手1失败")
        return False

    if handshake_2(serial_comm, handshake_data1, passcode):
        print("握手2成功")
    else:
        print("握手2失败")
        return False

    for start, stop in segments:
        size = stop - start
        if erase_flash(serial_comm, start, size):
            print(f"地址 {start:#010x} - {stop:#010x} 擦除成功")
        else:
            print(f"地址 {start:#010x} - {stop:#010x} 擦除失败")
            return False
        
        if write_flash(serial_comm, start, size, hex_data.tobinarray(start, stop)):
            print(f"地址 {start:#010x} - {stop:#010x} 写入成功")
        else:
            print(f"地址 {start:#010x} - {stop:#010x} 写入失败")
            return False
    
    if reset_chip(serial_comm):
        print("复位芯片成功")
    else:
        print("复位芯片失败")
        return False
    
    print("所有操作成功完成")
    return True

# 主函数
def main():
    # 使用 argparse 解析命令行参数
    parser = argparse.ArgumentParser(description="EZBOOT Programmer")
    parser.add_argument("port", type=str, help="串口端口 (例如: COM4 或 /dev/ttyUSB0)")
    parser.add_argument("hex_file", type=str, help="要烧写的 HEX 文件路径")
    parser.add_argument("passcode", type=str, help="passcode (例如: 010203040506)")
    parser.add_argument("--baudrate", type=int, default=115200, help="串口波特率 (默认: 115200)")
    args = parser.parse_args()

    # 设置串口连接
    port = args.port
    baudrate = args.baudrate
    try:
        serial_comm = SerialCommunication(port, baudrate)
    except serial.SerialException as e:
        print(f"无法打开串口 {port}: {e}", file=sys.stderr)
        sys.exit(1)

    # 解析 passcode
    try:
        passcode = bytes.fromhex(args.passcode)
    except ValueError:
        print("无效的 passcode 格式，应为十六进制字符串", file=sys.stderr)
        sys.exit(1)

    # 设置烧写的 hex 文件
    hex_file = args.hex_file

    # 读取 Intel Hex 文件
    try:
        hex_data = IntelHex(hex_file)
    except FileNotFoundError:
        print(f"找不到文件: {hex_file}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"读取 HEX 文件时出错: {e}", file=sys.stderr)
        sys.exit(1)

    # 获取所有的数据段（段的起始地址和结束地址）
    segments = list(hex_data.segments())

    # 打印出所有的段及其大小
    if segments:
        debug_print("Hex 文件包含数据段：")
        for start, stop in segments:
            size = stop - start
            debug_print(f"地址段: {start:#010x} - {stop:#010x}, 数据大小: {size} 字节")
    else:
        print("Hex 文件不包含任何数据段。")
        sys.exit(1)  # 如果没有数据段，认为是错误

    try:
        if not process_flash_operations(serial_comm, segments, passcode, hex_data):
            print("操作过程中遇到错误")
            sys.exit(1)  # 返回非零状态码表示失败
    finally:
        # 关闭串口
        serial_comm.close()
    
    # 成功完成所有操作
    sys.exit(0)

if __name__ == "__main__":
    main()