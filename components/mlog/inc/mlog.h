#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#include <boot_config.h>

#define LOG_OFF_LEVEL           0
#define LOG_ERR_LEVEL           1
#define LOG_WRN_LEVEL           2
#define LOG_INF_LEVEL           3
#define LOG_DBG_LEVEL           4


#define MLOG_LEVEL               CONFIG_LOG_LEVEL


void mlog_init(void);

void mlog_hex_d(char* string, uint8_t* buf, uint32_t size);

void mlog_hex_i(char* string, uint8_t* buf, uint32_t size);

void mlog_hex_w(char* string, uint8_t* buf, uint32_t size);

void mlog_hex_e(char* string, uint8_t* buf, uint32_t size);

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
