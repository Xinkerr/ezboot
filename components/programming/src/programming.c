/**
 * 
 * Copyright (c) 2025 Xinkerr
 * 
 * SPDX-License-Identifier: Apache-2.0
 *
 * Disclaimer / 免责声明
 *
 * This software is provided "as is", without warranty of any kind, express or implied, 
 * including but not limited to the warranties of merchantability, fitness for a 
 * particular purpose, or non-infringement. In no event shall the authors or copyright 
 * holders be liable for any claim, damages, or other liability, whether in an action 
 * of contract, tort, or otherwise, arising from, out of, or in connection with the 
 * software or the use or other dealings in the software.
 *
 * 本软件按“原样”提供，不附带任何明示或暗示的担保，包括但不限于对适销性、特定用途适用性
 * 或非侵权的保证。在任何情况下，作者或版权持有人均不对因本软件或使用本软件而产生的任何
 * 索赔、损害或其他责任负责，无论是合同诉讼、侵权行为还是其他情况。
 */

#include <ezboot_config.h>
#if CONFIG_PROGRAMMING_SUPPORT
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <mcu_header.h>
#include <boot.h>
#include <pg_uart_drv.h>
#include <tlv.h>
#include <programming.h>
#include <mlog.h>
#include <ezb_flash.h>
#include <uptime.h>
#include <tinycrypt/sha256.h>

#define DELAY_TIME_MS           10

#define PG_HEAD_L               0xaa
#define PG_HEAD_H               0x55

#define REQUEST_CONN_TAG        0x21
#define SHAKE_HAND_1_TAG        0x22
#define SHAKE_HAND_2_TAG        0x23
#define ERASE_TAG               0x24
#define WRITE_TAG               0x25
#define READ_TAG                0x26
#define RESET_TAG               0x27

#define CONN_RESP_TAG           0x41
#define SHAKE_HAND_1_RESP_TAG   0x42
#define SHAKE_HAND_2_RESP_TAG   0x43
#define ERASE_RESULT_TAG        0x44
#define WRITE_RESULT_TAG        0x45
#define READ_DATA_TAG           0x46
#define RESET_RESP_TAG          0x47

static bool connected_state = false;
static bool handshake1_flag = false;
static bool handshake_complete = false;

static uint8_t recv_buf[CONFIG_PG_RECV_BUF_SIZE];
static uint16_t recv_len = 0;
static tlv_t pg_payload_tlv;
static uint32_t shake_random;

static uint8_t passcode[] = CONFIG_PG_PASSCODE;

static int pg_connect_handler(uint8_t* value_data, uint16_t len);
static int pg_reset_handler(uint8_t* value_data, uint16_t len);
static int pg_erase_handler(uint8_t* value_data, uint16_t len);
static int pg_write_handler(uint8_t* value_data, uint16_t len);
static int pg_shake1_handler(uint8_t* value_data, uint16_t len);
static int pg_shake2_handler(uint8_t* value_data, uint16_t len);

static const tlv_tb_t pg_payload_tlv_tb[] = 
{
    {REQUEST_CONN_TAG, pg_connect_handler},
    {RESET_TAG, pg_reset_handler},
    {ERASE_TAG, pg_erase_handler},
    {WRITE_TAG, pg_write_handler},
    {SHAKE_HAND_1_TAG, pg_shake1_handler},
    {SHAKE_HAND_2_TAG, pg_shake2_handler},
};

static inline void pg_drv_init(void)
{
    #if CONFIG_PG_UART_USE
        pg_uart_drv_init();
    #endif
}

static inline void pg_drv_send(const void* pdata, uint16_t len)
{
    #if CONFIG_PG_UART_USE
        pg_uart_drv_send(pdata, len);
    #endif
}

static inline uint16_t pg_drv_get(uint8_t* pdata, uint16_t len)
{
    #if CONFIG_PG_UART_USE
        return pg_uart_drv_get(pdata, len);
    #else
        return 0;
    #endif
}

static uint16_t crc16_ccitt(uint16_t seed, const uint8_t *src, size_t len)
{
	for (; len > 0; len--) {
		uint8_t e, f;

		e = seed ^ *src;
		++src;
		f = e ^ (e << 4);
		seed = (seed >> 8) ^ ((uint16_t)f << 8) ^ ((uint16_t)f << 3) ^ ((uint16_t)f >> 4);
	}

	return seed;
}

static void data_left_move(uint8_t* pdata, uint16_t len, uint16_t move_step)
{
    int i;
    for(i=0; i<len-move_step; i++)
    {
        pdata[i] = pdata[i+move_step];
    }
}

static int pg_protocol_recv(void)
{
    uint16_t got_len = pg_drv_get(&recv_buf[recv_len], sizeof(recv_buf)-recv_len);
    recv_len += got_len;

    //检查帧头数据
    while(recv_len >= 2)
    {
        if(recv_buf[0] == PG_HEAD_L)
        {
            if(recv_buf[1] == PG_HEAD_H)
                break;
            else
            {
                data_left_move(recv_buf, recv_len, 2);
                recv_len -= 2;
            }
        }
        else
        {
            data_left_move(recv_buf, recv_len, 1);
            recv_len --;
        }
    }

    if(recv_len > 6)
    {
        uint16_t crc_res;
        uint16_t check_crc;
        int tlv_ret;
        crc_res = crc16_ccitt(0, recv_buf+2, 2);
        memcpy(&check_crc, recv_buf+4, sizeof(check_crc));
        //payload length校验
        if(crc_res == check_crc)
        {
            uint16_t payload_len;
            memcpy(&payload_len, recv_buf+2, sizeof(payload_len));
            uint8_t* payload_ptr = recv_buf+6;
            //[head size] + [length size] + [length check size] + [payload size] + [crc size]
            uint16_t sum_size = payload_len + 8; 
            //数据是否完整
            if(recv_len >= sum_size)
            {
                //整帧数据的CRC校验
                crc_res = crc16_ccitt(0, recv_buf, payload_len+6);
                memcpy(&check_crc, payload_ptr+payload_len, sizeof(check_crc));
                //payload data校验
                if(crc_res == check_crc)
                {
                    //解析TLV data
                    tlv_ret = tlv_parse(&pg_payload_tlv, payload_ptr, payload_len);
                    mlog_d("recv_len:%d", recv_len);
                    mlog_d("sum_size:%d", sum_size);
                    mlog_d("tlv_ret:%d", tlv_ret);
                    mlog_hex_d("data:", recv_buf, recv_len);
                    //从buffer中移除这部分的数据
                    data_left_move(recv_buf, recv_len, sum_size);
                    recv_len -= sum_size;
                    //返回执行结果
                    if(tlv_ret >= 0)
                        return 0;
                    else
                        return -2;
                }
                else
                {
                    mlog_e("payload crc err,0x%04x,0x%04x", check_crc, crc_res);
                    //可能这段数据里的head和payload len不是有效的; 也可能是payload data有错误
                    data_left_move(recv_buf, recv_len, 6);
                    recv_len -= 6;
                    return -1;
                }
            }
        }
        else
        {
            mlog_e("length crc err, 0x%04x,0x%04x", check_crc, crc_res);
            data_left_move(recv_buf, recv_len, 6);
            recv_len -= 6;
        }
    }

    return -1;
}

static uint16_t pg_protocol_format(uint8_t* pg_buf, uint8_t* payload_data, uint16_t payload_len)
{
    uint16_t crc_res;
    pg_buf[0] = PG_HEAD_L;
    pg_buf[1] = PG_HEAD_H;
    memcpy(pg_buf+2, &payload_len, 2);
    crc_res = crc16_ccitt(0, pg_buf+2, 2);
    memcpy(pg_buf+4, &crc_res, 2);
    memcpy(pg_buf+6, payload_data, payload_len);
    crc_res = crc16_ccitt(0, pg_buf, payload_len+6);
    memcpy(pg_buf+payload_len+6, &crc_res, 2);

    return payload_len+8;
}

static int wait_to_connect(void)
{
    uint32_t start_time = sys_uptime_get();
    uint32_t current_time = sys_uptime_get();
    while (current_time - start_time <= CONFIG_WAIT_TIME_FOR_PG)
    {
        //解析pg uart接收到的数据
        pg_protocol_recv();
        //检查是否建立连接
        if(connected_state)
            return 0;

        current_time = sys_uptime_get();
    }
	return -1;
}

void programming_process(void)
{
    pg_drv_init();

    pg_payload_tlv.tlv_table = (tlv_tb_t*)pg_payload_tlv_tb;
    pg_payload_tlv.tag_count = sizeof(pg_payload_tlv_tb)/sizeof(tlv_tb_t);

    mlog_i("wait to connect");
    if(wait_to_connect() == 0)
    {
        mlog_i("connected");
    }
    else
	{
		mlog_i("timeout, exit pg");
        return;
	}
    while(1)
    {
        pg_protocol_recv();
    }
}

static uint32_t random_generate(void)
{
    uint32_t random_value = sys_uptime_get();
    // mlog_d("uptime:%d", random_value);
    random_value = (random_value << 3)      // 左移操作扩大随机数
                    ^ (random_value >> 5)      // 右移操作增加随机性
                    ^ (random_value * 2654435761U); // 使用黄金分割常数混合
    // mlog_d("random_value:%u", random_value);

    return random_value;
}

static int pg_connect_handler(uint8_t* value_data, uint16_t len)
{
    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint8_t response_result = 0x01;
    uint16_t tlv_len;
    uint16_t send_len;

    //请求建立连接
    if(*value_data == 0x01)
    {
        mlog_i("request connect");
        connected_state = true;
    }
    //请求断开连接
    else if(*value_data == 0x02)
    {
        mlog_i("request disconnect");
        connected_state = false;
    }
    else
    {
        return -1;
    }

    //协议数据组包
    tlv_len = tlv_add(payload_buf, CONN_RESP_TAG, 1, &response_result);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);

    return 0;
}

static int pg_reset_handler(uint8_t* value_data, uint16_t len)
{
    if(!connected_state)
        return -1;

    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint8_t response_result = 0x00;
    uint16_t tlv_len;
    uint16_t send_len;

    tlv_len = tlv_add(payload_buf, RESET_RESP_TAG, 1, &response_result);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);
    
	uint32_t last = sys_uptime_get();
	uint32_t current = sys_uptime_get();
	while(current - last < 5)
	{
		current = sys_uptime_get();
	}
	mlog_i("reset chip");
	reboot();
    return 0;
}

static int pg_erase_handler(uint8_t* value_data, uint16_t len)
{
    mlog_d("%s", __FUNCTION__);
    if(!connected_state)
        return -1;
    if(!handshake_complete)
        return -1;

    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint8_t response_result;
    uint16_t tlv_len;
    uint16_t send_len;
    int ret;

    uint32_t erase_addr, erase_size;
    memcpy(&erase_addr, value_data, sizeof(erase_addr));
    memcpy(&erase_size, value_data+sizeof(erase_addr), sizeof(erase_size));
    mlog_d("erase addr:0x%x", erase_addr);
    mlog_d("erase size:%u", erase_size);
    
    if(erase_addr >= APP_ADDRESS)
    {
        if(ezb_flash_erase(erase_addr, erase_size) >= 0)
        {
            mlog_d("erase success");
            ret = 0;
        }
        else
        {
            mlog_d("erase fail");
            ret = -3;
        }
    }
    else
        ret = -2;

    if(ret == 0)
        response_result = 0x00;
    else
        response_result = 0xff;

    tlv_len = tlv_add(payload_buf, ERASE_RESULT_TAG, 1, &response_result);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);
    
    return ret;
}

static int pg_write_handler(uint8_t* value_data, uint16_t len)
{
    mlog_d("%s", __FUNCTION__);
   if(!connected_state)
       return -1;
   if(!handshake_complete)
       return -1;

    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint8_t response_result;
    uint16_t tlv_len;
    uint16_t send_len;
    int ret;

    uint32_t write_addr;
    uint16_t write_size;
    memcpy(&write_addr, value_data, sizeof(write_addr));
    memcpy(&write_size, value_data+sizeof(write_addr), sizeof(uint16_t));
    mlog_d("write addr:0x%x", write_addr);
    mlog_d("write size:%u", write_size);
    uint8_t* write_ptr = value_data + sizeof(write_addr) + sizeof(uint16_t);
    
    if(write_addr >= APP_ADDRESS)
    {
        if(ezb_flash_write(write_addr, write_ptr, write_size) >= 0)
        {
            mlog_d("write success");
            ret = 0;
        }
        else
        {
            mlog_d("write fail");
            ret = -3;
        }
    }
    else
        ret = -2;

    if(ret == 0)
        response_result = 0x00;
    else
        response_result = 0xff;

    tlv_len = tlv_add(payload_buf, WRITE_RESULT_TAG, 1, &response_result);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);
    
    return ret;
}

static int pg_shake1_handler(uint8_t* value_data, uint16_t len)
{
    mlog_d("%s", __FUNCTION__);
    if(!connected_state)
        return -1;

    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint16_t tlv_len;
    uint16_t send_len;

    shake_random = random_generate();
    mlog_hex_d("shake:", &shake_random, 4);

    tlv_len = tlv_add(payload_buf, SHAKE_HAND_1_RESP_TAG, 4, &shake_random);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);

    handshake1_flag = true;

    return 0;
}

static int pg_shake2_handler(uint8_t* value_data, uint16_t len)
{
    mlog_d("%s", __FUNCTION__);
    if(!connected_state)
        return -1;
    if(!handshake1_flag)
        return -1;

    uint8_t send_buf[32];
    uint8_t payload_buf[8];
    uint8_t response_result;
    uint16_t tlv_len;
    uint16_t send_len;
    int ret;

    if(len == 32)
    {
        uint8_t* shakehand_data_ptr = value_data;
        struct tc_sha256_state_struct sha256_s;
        uint8_t sha256_result[32];
        uint8_t merge[4+sizeof(passcode)];

        memcpy(merge, &shake_random, 4);
        memcpy(merge+4, passcode, sizeof(passcode));

        tc_sha256_init(&sha256_s);
        tc_sha256_update(&sha256_s, merge, sizeof(merge));
        tc_sha256_final(sha256_result, &sha256_s);
        mlog_hex_d("SHA256:", sha256_result, 32);

        if(memcmp(sha256_result, shakehand_data_ptr, 32) == 0)
        {
            mlog_d("shake hand complete");
            response_result = 0;
            ret = 0;
            handshake_complete = true;
        }
        else
        {
            mlog_d("shake hand 2 fail");
            response_result = 0xff;
            ret = -2;
        }
    }
    else
    {
        response_result = 0xff;
        ret = -2;
    }

    tlv_len = tlv_add(payload_buf, SHAKE_HAND_2_RESP_TAG, 1, &response_result);
    send_len = pg_protocol_format(send_buf, payload_buf, tlv_len);
    pg_drv_send(send_buf, send_len);
    
    return ret;
}
#endif
