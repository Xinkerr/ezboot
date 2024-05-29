#include <image_decrypt.h>
#if CONFIG_OTA_IMAGE_AES128_ENCRYPT
#include <string.h>
#include <tinycrypt/aes.h>
#include <tinycrypt/sha256.h>
#include <tinycrypt/cbc_mode.h>
#include <mlog.h>
#include <tinycrypt/constants.h>

const static uint8_t aes_key[16] = CONFIG_OTA_IMAGE_AES_KEY;
const static uint8_t aes_iv[16] = CONFIG_OTA_IMAGE_AES_IV;

static uint8_t decrypt_key[16];
static uint8_t decrypt_iv[16];
static uint8_t decrypting_cache[CONFIG_OTA_WRITE_BLOCK_MAX+16];

static struct tc_aes_key_sched_struct tc_aes_sched;
int image_decrtpy_key_init(const uint8_t* salt, uint16_t len)
{
    struct tc_sha256_state_struct sha256_s;
    uint8_t sha256_result[32];
    uint8_t merge[32];

    if(salt == NULL || len == 0)
        return -1;

    memcpy(merge, aes_key, sizeof(aes_key));
    memcpy(merge + sizeof(aes_key), salt, len);
    mlog_hex_d("merge key: ", merge, sizeof(merge));

    tc_sha256_init(&sha256_s);
    tc_sha256_update(&sha256_s, merge, sizeof(merge));
    tc_sha256_final(sha256_result, &sha256_s);

    memcpy(decrypt_key, &sha256_result[8], 16);
    mlog_hex_d("decrypt_key: ", decrypt_key, sizeof(decrypt_key));

    return 0;
}

int image_decrypt_iv_init(const uint8_t* salt, uint16_t len)
{
    struct tc_sha256_state_struct sha256_s;
    uint8_t sha256_result[32];
    uint8_t merge[32];

    if(salt == NULL || len == 0)
        return -1;

    memcpy(merge, aes_iv, sizeof(aes_iv));
    memcpy(merge + sizeof(aes_iv), salt, len);
    mlog_hex_d("merge iv: ", merge, sizeof(merge));

    tc_sha256_init(&sha256_s);
    tc_sha256_update(&sha256_s, merge, sizeof(merge));
    tc_sha256_final(sha256_result, &sha256_s);

    memcpy(decrypt_iv, &sha256_result[8], 16);
    mlog_hex_d("decrypt_iv: ", decrypt_iv, sizeof(decrypt_iv));

    return 0;
}

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
                        const uint8_t* iv_salt, uint16_t is_len)
{
    // 检查传入的参数是否有效
    if(key_salt == NULL || ks_len == 0 || iv_salt == NULL || is_len == 0)
        return -1;

    // 初始化解密密钥
    image_decrtpy_key_init(key_salt, ks_len);
    // 初始化解密IV
    image_decrypt_iv_init(iv_salt, is_len); 

    // 设置AES128加密密钥，准备解密
    tc_aes128_set_encrypt_key(&tc_aes_sched, decrypt_key);

    return 0;
}


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
int image_decrypt_data(uint8_t *in_data, uint32_t len, uint8_t* ou_data)
{
    // 检查输入参数是否有效
    if(in_data == NULL || len == 0 || ou_data == NULL)
        return -1;

    // mlog_hex_d("encrypt data: ", in_data, len);
    // 初始化解密缓存，将解密向量复制到缓存的开始部分，然后将输入数据复制到缓存的剩余部分
    memcpy(decrypting_cache, decrypt_iv, sizeof(decrypt_iv));
    memcpy(decrypting_cache+sizeof(decrypt_iv), in_data, len);
    // mlog_hex_d("decrypting_cache: ", decrypting_cache, len+sizeof(decrypt_iv));
    
    // 使用AES-CBC模式对数据进行解密
    tc_cbc_mode_decrypt(ou_data, len, 
                        in_data, len, 
                        decrypting_cache, &tc_aes_sched);  
    // 更新解密向量，将其设置为解密后数据的最后TC_AES_BLOCK_SIZE字节
    memcpy(decrypt_iv, decrypting_cache+len, TC_AES_BLOCK_SIZE);
    // mlog_hex_d("decrypt_iv: ", decrypt_iv, 16);
    
    // 记录解密后的数据，主要用于调试和日志记录
    mlog_hex_d("decrypt data: ", ou_data, len);

    return 0;
}

#endif //CONFIG_OTA_IMAGE_AES128_ENCRYPT

