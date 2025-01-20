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

#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>

typedef enum 
{
    TLV_OK = 0,
    TLV_TAG_NOT_FOUND = -1,
    TLV_SIZE_SMALL = -2,
    TLV_DATA_INCOMPLETE = -3,
    TLV_HANDLE_ERROR = -4
}tlv_err_t;

typedef struct
{
    uint8_t tag;
    void (*handler)(uint8_t* value_data, uint16_t len);
}tlv_tb_t;

typedef struct 
{
    tlv_tb_t* tlv_table;
    uint16_t tag_count;
}tlv_t;

tlv_err_t tlv_parse(tlv_t* tlv, uint8_t* pdata, uint16_t pdata_size);

#endif
