#ifndef __IMAGE_DECRYPT_H__
#define __IMAGE_DECRYPT_H__

#include <stdint.h>
#include <boot_config.h>
#if CONFIG_OTA_IMAGE_AES128_ENCRYPT

/**
 * 初始化image解密过程。
 * 使用给定的密钥盐和IV盐来初始化解密所需的密钥和IV。
 * 
 * @param key_salt 密钥盐的指针，用于生成解密密钥。
 * @param ks_len 密钥盐的长度。
 * @param iv_salt IV盐的指针，用于生成解密中使用的IV。
 * @param is_len IV盐的长度。
 * @return 成功返回0，任一参数无效返回-1。
 */
int image_decrypt_init(const uint8_t* key_salt, uint16_t ks_len,
                        const uint8_t* iv_salt, uint16_t is_len);

/**
 * 分块解密image数据
 * 
 * 本函数用于对给定的image的块数据进行解密操作。
 * 采用CBC模式的AES算法进行解密，并将解密后的数据存储到指定的输出缓冲区。
 * 
 * @param in_data 指向需要解密的块数据的指针。
 * @param len 需要解密的块数据的长度。
 * @param ou_data 指向用于存储解密后数据的输出缓冲区的指针。
 * @return 函数执行成功返回0，输入参数无效返回-1。
 */
int image_decrypt_data(const uint8_t *in_data, uint32_t len, uint8_t* ou_data);

#endif //CONFIG_OTA_IMAGE_AES128_ENCRYPT

#endif
