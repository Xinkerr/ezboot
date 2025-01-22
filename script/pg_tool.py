import struct
from typing import List

def reverse_bits(value: int, bit_width: int = 16) -> int:
    """反转指定宽度的位"""
    reversed_value = 0
    for _ in range(bit_width):
        reversed_value = (reversed_value << 1) | (value & 1)
        value >>= 1
    return reversed_value

def crc16_ccitt(data: bytes, seed: int = 0x0000) -> int:
    """计算 CRC16 CCITT 校验码 (初始值为 0x0000，输入和输出均需反转)"""
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

def generate_frame(tag: int, value_length: int, value: List[int]) -> bytes:
    """
    根据协议生成数据帧。

    :param tag: TLV 的 Tag 值 (1 byte)
    :param value_length: TLV 的 Value 长度
    :param value: TLV 的 Value 值 (list of bytes)
    :return: 生成的数据帧 (bytes)
    """
    # TLV 数据
    tlv_data = struct.pack("<B", tag) + struct.pack("<H", value_length) + bytes(value)

    # 帧头部 (HEAD: 0x55AA)
    head = 0x55AA

    # Payload 数据包括 TLV 数据
    payload_length = len(tlv_data)

    # Length Check: 对 Payload Length 计算 CRC16
    length_check = crc16_ccitt(struct.pack("<H", payload_length))

    # 整个帧的数据 (HEAD + PAYLOAD LENGTH + LENGTH CHECK + TLV DATA)
    frame_without_crc = (
        struct.pack("<H", head) +
        struct.pack("<H", payload_length) +
        struct.pack("<H", length_check) +
        tlv_data
    )

    # CRC 校验 (对帧的前面所有数据计算 CRC16)
    crc = crc16_ccitt(frame_without_crc)

    # 最终帧 (加上 CRC 校验码)
    frame = frame_without_crc + struct.pack("<H", crc)

    return frame

def main():
    print("请输入 Tag、Value 长度和 Value (Value 用十六进制表示并以空格分隔，例如: 01 02 03)：")

    # 输入 Tag
    tag = int(input("Tag (十六进制): "), 16)

    # 输入 Value 长度
    value_length = int(input("Value 长度 (十进制): "))

    # 输入 Value
    if value_length > 0:
        value_input = input("Value (十六进制，用空格分隔): ").strip()
        value = [int(x, 16) for x in value_input.split()]
        if len(value) != value_length:
            print("错误: 输入的 Value 长度与指定的长度不匹配！")
            return
    else:
        value = []

    # 生成数据帧
    frame = generate_frame(tag, value_length, value)

    # 输出生成的帧
    print("生成的数据帧 (十六进制):")
    print(" ".join(f"{byte:02X}" for byte in frame))

if __name__ == "__main__":
    main()
