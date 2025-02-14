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

#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#include <stdint.h>
#include <ezboot_config.h>

#define LOG_OFF_LEVEL           0
#define LOG_ERR_LEVEL           1
#define LOG_WRN_LEVEL           2
#define LOG_INF_LEVEL           3
#define LOG_DBG_LEVEL           4


#define MLOG_LEVEL               CONFIG_LOG_LEVEL


void mlog_init(void);

void mlog_hex_d(char* string, const void* buf, uint32_t size);

void mlog_hex_i(char* string, const void* buf, uint32_t size);

void mlog_hex_w(char* string, const void* buf, uint32_t size);

void mlog_hex_e(char* string, const void* buf, uint32_t size);

#if MLOG_LEVEL >= LOG_DBG_LEVEL
#define mlog_d(format, ...) printf("D: " format "\r\n", ##__VA_ARGS__)
#else
#define mlog_d(...)
#endif

#if MLOG_LEVEL >= LOG_INF_LEVEL
#define mlog_i(format, ...) printf("I: " format "\r\n", ##__VA_ARGS__)
#else
#define mlog_i(...)
#endif

#if MLOG_LEVEL >= LOG_WRN_LEVEL
#define mlog_w(format, ...) printf("W: " format "\r\n", ##__VA_ARGS__)
#else
#define mlog_w(...)
#endif

#if MLOG_LEVEL >= LOG_ERR_LEVEL
#define mlog_e(format, ...) printf("E: " format "\r\n", ##__VA_ARGS__)
#else
#define mlog_e(...)
#endif

#if MLOG_LEVEL > LOG_OFF_LEVEL
#define mlog(format, ...) printf(format, ##__VA_ARGS__)
#else
#define mlog(...)
#endif

#endif
