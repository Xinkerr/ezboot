# 编程功能

ezboot 支持通过串口编程来烧写 APP 固件。你可以在代码中启用配置，然后通过 `programmer.py` 工具进行固件擦除和写入。

## 配置项说明

在 `ezboot_config.h` 文件中进行如下配置：

### 启用串口编程功能

```
#define CONFIG_PROGRAMMING_SUPPORT 1       // 启用编程功能
#define CONFIG_PG_UART_USE 1               // 启用串口编程
```

### Buffer 大小设置

| 宏名称                    | 说明                                             |
| ------------------------- | ------------------------------------------------ |
| `CONFIG_PG_UART_RB_SIZE`  | 串口接收的环形缓冲区大小（建议 > 4096 + 17）     |
| `CONFIG_PG_RECV_BUF_SIZE` | 编程协议解析接收的缓冲区大小（建议 > 4096 + 17） |

### APP 启动延迟

| 宏名称                    | 说明                                                     |
| ------------------------- | -------------------------------------------------------- |
| `CONFIG_WAIT_TIME_FOR_PG` | APP 延迟启动，等待与 programmer 工具建立连接，单位：毫秒 |

### 设置 PASSCODE

| 宏名称               | 说明                                                         |
| -------------------- | ------------------------------------------------------------ |
| `CONFIG_PG_PASSCODE` | 设置访问密码，programmer 工具输入的密码需与此匹配，否则无法完成编程 |

## 配置示例

在 `ezboot_config.h` 中进行以下配置：

```C
#define CONFIG_PROGRAMMING_SUPPORT  1           // 启用编程支持
#define CONFIG_PG_UART_USE          1           // 启用串口编程
#define CONFIG_PG_UART_RB_SIZE      5000        // 设置串口接收环形缓冲区大小
#define CONFIG_PG_RECV_BUF_SIZE     5000        // 设置接收缓冲区大小
#define CONFIG_WAIT_TIME_FOR_PG     800         // 设置等待时间：800ms
#define CONFIG_PG_PASSCODE          {0x01, 0x02, 0x03, 0x04, 0x05, 0x06} // 设置PASSCODE
```

## 操作说明

### 1. 配置编程功能

按照上述配置启用编程功能。

### 2. 安装 `programmer` 工具环境

首先确保电脑已安装 Python。然后，在 `script` 文件夹中打开终端，输入以下命令安装所需依赖：

```bash
pip install -r requirements.txt
```

### 3. 使用 `programmer.py` 工具烧写固件

在 `script` 文件夹中打开终端，执行 `programmer.py` 脚本来烧写固件。命令需要带上以下参数：

- **COM口**：指定串口
- **HEX 文件**：待烧写的固件文件
- **Passcode**：与 `ezboot_config.h` 中的 `CONFIG_PG_PASSCODE` 一致的密码

示例命令：

```bash
python programmer.py COM3 firmware.hex 010203040506
```

此命令将会通过指定的 COM 端口烧写固件。

注意需要做好分区，烧写的目标地址不能与ezboot的区域重合！