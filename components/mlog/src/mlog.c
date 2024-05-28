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
