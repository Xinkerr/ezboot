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
#include <mcu_header.h>
#include <ezboot_config.h>
#include <pg_uart_drv.h>
#include <delay.h>
#include <tlv.h>
#include <programming.h>

#define DELAY_TIME_MS       10

static uint8_t recv_buf[512];
static uint16_t recv_len = 0;
static tlv_t pg_tlv;

static pg_payload_handler(uint8_t* value_data, uint16_t len);

static const tlv_tb_t pg_tlv_tb[] = 
{
    {0x01, pg_payload_handler},
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

static int wait_to_connect(void)
{
    uint32_t wait_time = 0;
    while (wait_time <= CONFIG_WAIT_TIME_FOR_PG)
    {
        //解析pg uart接收到的数据
        delay_ms(DELAY_TIME_MS);
        wait_time += DELAY_TIME_MS;
    }
}

static void data_left_move(uint8_t* pdata, uint16_t len)
{
    int i;
    for(i=0; i<len; i++)
    {
        
    }
    
}

void programming_process(void)
{
    pg_drv_init();
    pg_tlv.tlv_table = pg_tlv_tb;
    pg_tlv.tag_count = sizeof(pg_tlv_tb)/sizeof(tlv_tb_t);

    if(wait_to_connect() != 0)
        return;
    while(1)
    {
        uint16_t got_len = pg_drv_get(&recv_buf[recv_len], sizeof(recv_buf)-recv_len);
        recv_len += got_len;
        //解析pg uart接收到的数据
        tlv_err_t err = tlv_parse(&pg_tlv, recv_buf, recv_len);
        while(err == TLV_TAG_NOT_FOUND || err == TLV_HANDLE_ERROR)
        {
            recv_buf
            err = tlv_parse(&pg_tlv, recv_buf, recv_len);
        }
        switch (err)
        {
        case TLV_OK:
            recv_len = 0;
            memset(recv_buf, 0, sizeof(recv_buf));
            break;

        case TLV_TAG_NOT_FOUND:
        
        default:
            break;
        }
    }
}

