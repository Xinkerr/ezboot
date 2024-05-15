import intelhex
import struct
import os
import sys
import binascii
import argparse
import time
import random

# 从 config.h 文件中读取参数
with open('config.h', 'r') as f:
    for line in f:
        if line.startswith('#define CONFIG_DATA_VERSION'):
            DATA_VERSION = int(line.split()[-1], 16)
        elif line.startswith('#define CONFIG_OTA_VERSION'):
            OTA_VERSION = int(line.split()[-1], 16)
            OTA_VERSION_MAJOR = (OTA_VERSION >> 16) & 0xFFFF
            OTA_VERSION_MINOR = OTA_VERSION & 0xFFFF

# 定义头部信息参数
HEADER_LENGTH = 128
HEAD_DATA = b'OTAB' 

def convert_hex_to_bin(hex_file, bin_file):
    # 读取 Intel HEX 文件
    ih = intelhex.IntelHex(hex_file)
    bin_data = ih.tobinarray()

    # 计算 CRC32 校验和
    crc32 = binascii.crc32(bin_data) & 0xFFFFFFFF

    # 计算 bin 文件大小
    file_size = len(bin_data)

    # 生成随机盐和AES IV
    random_salt = os.urandom(16)
    print(f"Random Salt: {random_salt.hex()}")
    aes_iv = os.urandom(16)
    print(f"AES IV: {aes_iv.hex()}")

    # 构建头部信息
    header_items = [
        HEAD_DATA,
        struct.pack('I', HEADER_LENGTH),
        struct.pack('I', DATA_VERSION),
        struct.pack('I', OTA_VERSION),
        struct.pack('I', file_size),
        struct.pack('I', crc32),
        random_salt,
        aes_iv
    ]
    header_size = sum(len(item) for item in header_items)
    header = b''.join(header_items)
    padding_size = HEADER_LENGTH - header_size
    header += b'\x00' * padding_size

    # 在数据前添加头部信息
    full_data = header + bin_data

    # 保存到 bin 文件
    with open(bin_file, 'wb') as f:
        f.write(full_data)

    print(f"Converted {hex_file} to {bin_file} with header information.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert Intel HEX to OTA binary with header')
    parser.add_argument('hex_file', help='Input Intel HEX file')
    args = parser.parse_args()

    hex_file = args.hex_file
    # bin_file = os.path.splitext(hex_file)[0] + ".bin"
    # 生成输出文件名
    bin_file = os.path.splitext(hex_file)[0] + f"_v{OTA_VERSION_MAJOR:X}.{OTA_VERSION_MINOR:X}.bin"
    convert_hex_to_bin(hex_file, bin_file)
