"""
SPDX-License-Identifier: Apache-2.0

Disclaimer / 免责声明

This software is provided "as is", without warranty of any kind, express or implied, 
including but not limited to the warranties of merchantability, fitness for a 
particular purpose, or non-infringement. In no event shall the authors or copyright 
holders be liable for any claim, damages, or other liability, whether in an action 
of contract, tort, or otherwise, arising from, out of, or in connection with the 
software or the use or other dealings in the software.

本软件按“原样”提供，不附带任何明示或暗示的担保，包括但不限于对适销性、特定用途适用性
或非侵权的保证。在任何情况下，作者或版权持有人均不对因本软件或使用本软件而产生的任何
索赔、损害或其他责任负责，无论是合同诉讼、侵权行为还是其他情况。
"""

import intelhex
import struct
import os
import sys
import binascii
import argparse
import time
import random
from Crypto.Cipher import AES
from Crypto.Hash import SHA256

def _pad_data(data, block_size):
    padding_length = block_size - (len(data) % block_size)
    padding = bytes([padding_length]) * padding_length
    return data + padding

# 从 config.h 文件中读取参数
with open('ezboot_config.h', 'r') as f:
    for line in f:
        if line.startswith('#define CONFIG_DATA_VERSION'):
            DATA_VERSION = int(line.split()[-1], 16)
        elif line.startswith('#define CONFIG_OTA_VERSION'):
            OTA_VERSION = int(line.split()[-1], 16)
            OTA_VERSION_MAJOR = (OTA_VERSION >> 16) & 0xFFFF
            OTA_VERSION_MINOR = OTA_VERSION & 0xFFFF
        elif line.startswith('#define CONFIG_OTA_IMAGE_AES_KEY'):
            AES_KEY = bytes([int(x, 16) for x in line.split('{')[1].split('}')[0].split(', ')])
        elif line.startswith('#define CONFIG_OTA_IMAGE_AES_IV'):
            AES_IV = bytes([int(x, 16) for x in line.split('{')[1].split('}')[0].split(', ')])
        elif line.startswith('#define CONFIG_OTA_IMAGE_AES128_ENCRYPT'):
            AES128_ENCRYPT = int(line.split()[-1])

# 定义头部信息参数
HEADER_LENGTH = 128
HEAD_DATA = b'OTAB'

def convert_hex_to_bin(hex_file, bin_file, encrypted_bin_file):
    # 读取 Intel HEX 文件
    ih = intelhex.IntelHex(hex_file)
    bin_data = bytes(ih.tobinarray())

    # 计算 bin 文件大小
    file_size = len(bin_data)

    if AES128_ENCRYPT:
        # 生成KEY盐和IV盐
        aes_key_salt = os.urandom(16)
        print(f"AES Key salt: {aes_key_salt.hex()}")
        aes_iv_salt = os.urandom(16)
        print(f"AES IV salt: {aes_iv_salt.hex()}")

        # 计算加密的 AES Key 和 IV
        aes_key = SHA256.new(AES_KEY + aes_key_salt).digest()[8:24]
        print(f"aes_key: {aes_key.hex()}")
        aes_iv = SHA256.new(AES_IV + aes_iv_salt).digest()[8:24]
        print(f"aes_iv: {aes_iv.hex()}")

        # 加密 bin_data
        cipher = AES.new(aes_key, AES.MODE_CBC, aes_iv)
        padded_data = _pad_data(bin_data, AES.block_size)
        encrypted_data = cipher.encrypt(padded_data)

        # 计算 CRC32 校验和
        crc32 = binascii.crc32(encrypted_data) & 0xFFFFFFFF
        print(f"encrypted data length {len(encrypted_data)} bytes")

        # 构建头部信息
        header_items = [
            HEAD_DATA,
            struct.pack('I', HEADER_LENGTH),
            struct.pack('I', DATA_VERSION),
            struct.pack('I', OTA_VERSION),
            struct.pack('I', file_size),
            struct.pack('I', crc32),
            struct.pack('I', len(encrypted_data)),
            aes_key_salt,
            aes_iv_salt
        ]
        header_size = sum(len(item) for item in header_items)
        header = b''.join(header_items)
        padding_size = HEADER_LENGTH - header_size
        header += b'\x00' * padding_size

        # 在加密数据前添加头部信息
        full_data = header + encrypted_data

        # 保存到 ota 文件
        with open(bin_file, 'wb') as f:
            f.write(full_data)

        # 保存加密数据到单独的文件
        if encrypted_bin_file:
            with open(encrypted_bin_file, 'wb') as f:
                f.write(encrypted_data)
    else:
        # 计算 CRC32 校验和
        crc32 = binascii.crc32(bin_data) & 0xFFFFFFFF

        # 构建头部信息
        header_items = [
            HEAD_DATA,
            struct.pack('I', HEADER_LENGTH),
            struct.pack('I', DATA_VERSION),
            struct.pack('I', OTA_VERSION),
            struct.pack('I', file_size),
            struct.pack('I', crc32)
        ]
        header_size = sum(len(item) for item in header_items)
        header = b''.join(header_items)
        padding_size = HEADER_LENGTH - header_size
        header += b'\x00' * padding_size

        # 在未加密数据前添加头部信息
        full_data = header + bin_data

        # 保存到 ota 文件
        with open(ota_file, 'wb') as f:
            f.write(full_data)

    print(f"Converted {hex_file} to {bin_file} with header information{' and AES encryption' if AES128_ENCRYPT else ''}.")
    if encrypted_bin_file:
        print(f"Converted {hex_file} to {encrypted_bin_file} with AES encryption.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert Intel HEX to OTA binary with header and AES encryption')
    parser.add_argument('hex_file', help='Input Intel HEX file')
    parser.add_argument('--generate-encrypted-bin', action='store_true', help='Generate AES-encrypted APP binary file')
    args = parser.parse_args()

    hex_file = args.hex_file
    ota_file = os.path.splitext(hex_file)[0] + f"_v{OTA_VERSION_MAJOR:X}.{OTA_VERSION_MINOR:X}.ota"
    encrypted_bin_file = os.path.splitext(hex_file)[0] + f"_v{OTA_VERSION_MAJOR:X}.{OTA_VERSION_MINOR:X}_encrypted.bin" if args.generate_encrypted_bin else None
    convert_hex_to_bin(hex_file, ota_file, encrypted_bin_file)
