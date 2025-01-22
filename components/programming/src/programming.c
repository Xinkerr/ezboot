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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <mcu_header.h>
#include <boot.h>
#include <ezboot_config.h>
#include <pg_uart_drv.h>
#include <delay.h>
#include <tlv.h>
#include <programming.h>
#if CONFIG_LOG_LEVEL
#include <mlog.h>
#endif

#define DELAY_TIME_MS       10

#define PG_HEAD_L               0xaa
#define PG_HEAD_H               0x55

static bool connected_state = false;

static uint8_t recv_buf[512];
static uint16_t recv_len = 0;
static tlv_t pg_payload_tlv;

static int pg_connect_handler(uint8_t* value_data, uint16_t len);
static int pg_reset_handler(uint8_t* value_data, uint16_t len);

static const tlv_tb_t pg_payload_tlv_tb[] = 
{
    {0x21, pg_connect_handler},
    {0x27, pg_reset_handler}
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
        check_crc = *(uint16_t*)(recv_buf+4);
        //payload length校验
        if(crc_res == check_crc)
        {
            uint16_t payload_len = *(uint16_t*)(recv_buf+2);
            uint8_t* payload_ptr = recv_buf+6;
            //[head size] + [length size] + [length check size] + [payload size] + [crc size]
            uint16_t sum_size = payload_len + 8; 
            //数据是否完整
            if(recv_len >= sum_size)
            {
                //整帧数据的CRC校验
                crc_res = crc16_ccitt(0, recv_buf, payload_len+6);
                check_crc = *(uint16_t*)(payload_ptr+payload_len);
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

static int wait_to_connect(void)
{
    uint32_t wait_time = 0;
    while (wait_time <= CONFIG_WAIT_TIME_FOR_PG)
    {
        //解析pg uart接收到的数据
        pg_protocol_recv();
        //检查是否建立连接
        if(connected_state)
            return 0;

        delay_ms(DELAY_TIME_MS);
        wait_time += DELAY_TIME_MS;
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

static int pg_connect_handler(uint8_t* value_data, uint16_t len)
{
    if(*value_data == 0x01)
    {
        mlog_i("request connect");
        connected_state = true;
    }
    else if(*value_data == 0x02)
    {
        mlog_i("request disconnect");
        connected_state = false;
    }

    return 0;
}

static int pg_reset_handler(uint8_t* value_data, uint16_t len)
{
    if(!connected_state)
        return -1;
        
    mlog_i("reset chip");
    reboot();
    return 0;
}
