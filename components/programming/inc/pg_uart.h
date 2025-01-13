#ifndef __PG_UART_H__
#define __PG_UART_H__

#include <stdint.h>

/**
 * @file pg_uart.h
 * @brief UART模块接口定义文件
 *
 * 提供UART初始化、发送数据以及注册接收回调函数的接口。
 * 用户可以通过本模块实现UART通信，并通过注册回调函数处理接收到的数据。
 */

/**
 * @typedef recv_callback_t
 * @brief UART接收回调函数类型定义
 *
 * @param recv_buf 接收到的数据缓冲区指针
 * @param len 接收到的数据长度
 *
 * 用户需要实现符合此类型签名的回调函数，并通过 `pg_uart_register_rx_callback` 注册。
 */
typedef void (*recv_callback_t)(void* recv_buf, uint16_t len);

/**
 * @brief UART初始化函数
 *
 * 初始化UART硬件模块和相关资源，使能UART通信。
 *
 * @return int
 *   - 0: 初始化成功
 *   - 非0: 初始化失败（具体错误码因实现而异）
 */
int pg_uart_init(void);

/**
 * @brief UART数据发送函数
 *
 * 向UART发送指定长度的数据。
 *
 * @param buf 数据缓冲区指针，存放待发送的数据
 * @param len 数据长度，单位：字节
 *
 * 调用此函数后，数据将通过UART发送。
 */
void pg_uart_send(void* buf, uint16_t len);

/**
 * @brief 注册UART接收回调函数
 *
 * 注册一个回调函数，在接收到UART数据时会自动调用该函数。
 *
 * @param callback 用户定义的接收回调函数，符合 `recv_callback_t` 类型签名
 *
 * 通过此接口，用户可以指定自定义的接收处理逻辑。当UART接收到数据时，模块将调用此回调函数并传递接收到的数据缓冲区和长度。
 */
void pg_uart_register_rx_callback(recv_callback_t callback);

#endif // __PG_UART_H__