#include <stdint.h>
#include <mlog.h>

extern void log_uart_init(void);

__inline void mlog_init(void) 
{
    log_uart_init();
}

__inline void mlog_hex_d(char* string, uint8_t* buf, uint32_t size)
{
    printf("D:%s", string);
    for(int i=0; i<size; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\r\n");
}
