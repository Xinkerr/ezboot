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
#include <mlog.h>

#if MLOG_LEVEL > LOG_OFF_LEVEL
extern void log_uart_init(void);

__inline void mlog_init(void) 
{
    log_uart_init();
}
#else
__inline void mlog_init(void) {}
#endif

#if MLOG_LEVEL >= LOG_DBG_LEVEL
__inline void mlog_hex_d(char* string, const void* buf, uint32_t size)
{
    printf("D: %s", string);
    for(int i=0; i<size; i++)
    {
        printf("%02x", *((uint8_t*)buf+i));
    }
    printf("\r\n");
}
#else
__inline void mlog_hex_d(char* string, const void* buf, uint32_t size) {}
#endif

#if MLOG_LEVEL >= LOG_INF_LEVEL
__inline void mlog_hex_i(char* string, const void* buf, uint32_t size)
{
    printf("I: %s", string);
    for(int i=0; i<size; i++)
    {
        printf("%02x", *((uint8_t*)buf+i));
    }
    printf("\r\n");
}
#else
__inline void mlog_hex_i(char* string, const void* buf, uint32_t size) {}
#endif

#if MLOG_LEVEL >= LOG_WRN_LEVEL
__inline void mlog_hex_w(char* string, const void* buf, uint32_t size)
{
    printf("W: %s", string);
    for(int i=0; i<size; i++)
    {
        printf("%02x", *((uint8_t*)buf+i));
    }
    printf("\r\n");
}
#else
__inline void mlog_hex_w(char* string, const void* buf, uint32_t size) {}
#endif

#if MLOG_LEVEL >= LOG_ERR_LEVEL
__inline void mlog_hex_e(char* string, const void* buf, uint32_t size)
{
    printf("E: %s", string);
    for(int i=0; i<size; i++)
    {
        printf("%02x", *((uint8_t*)buf+i));
    }
    printf("\r\n");
}
#else
__inline void mlog_hex_e(char* string, const void* buf, uint32_t size) {}
#endif
