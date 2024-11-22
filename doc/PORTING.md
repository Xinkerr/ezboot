# EZBOOT移植

## 移植操作方法

1. 新建MCU的模板工程。

2. 将ezboot的main.c和components里的文件添加到工程里。

3. 在工程下分别新增config和porting文件夹，并分别在文件夹下新建以下移植文件：

   | 文件                            | 说明                                                  |
   | ------------------------------- | ----------------------------------------------------- |
   | config/ezboot_config.h          | EZBOOT的配置头文件                                    |
   | porting/ezb_flash/ezb_flash.c   | 调用片内flash的擦除读写等函数，适配ezb_flash接口      |
   | porting/mcu_header/mcu_header.h | 添加MCU所有必要的头文件，如"stm32f1xx_hal.h"          |
   | porting/mlog/mlog_uart.c        | 适配mlog的接口，实现串口打印                          |
   | porting/norflash/norflash_spi.c | 适配norflash_spi.h接口，实现norflash依赖的SPI传输功能 |

   实现以上移植接口的功能。具体可以参考project路径下的工程样例。

5. 移植完成后，若实现了打印功能并且日志等级不为0，那么设备上电后则能看到EZBOOT大字样的打印。

## 配置项说明

配置项主要是在ezboot_config.h头文件中以宏定义的方式进行配置。配置项信息如下：

| 配置项名称                       | 含义                                                         |
| -------------------------------- | ------------------------------------------------------------ |
| CONFIG_OTA_IMAGE_AES128_ENCRYPT  | 是否启用OTA镜像加密功能。                                    |
| CONFIG_OTA_WRITE_BLOCK_MAX       | 每次往APP区写入OTA固件时的最大数据量。                       |
| CONFIG_OTA_IMAGE_AES_KEY         | OTA的AES加密的key                                            |
| CONFIG_OTA_IMAGE_AES_IV          | OTA的AES加密的IV                                             |
| CONFIG_NORFLASH_CMD_SECTOR_ERASE | norflash的擦除命令。若不定义则使用默认值0x20                 |
| CONFIG_NORFLASH_ERASE_SIZE       | norflash的擦除大小。若不定义则使用默认值4096                 |
| CONFIG_NORFLASH_PAGE_SIZE        | norflash的页大小。若不定义则使用默认值256                    |
| BOOT_ADDRESS                     | boot分区的起始地址                                           |
| APP_ADDRESS                      | app分区的起始地址                                            |
| APP_REGION_SIZE                  | app分区的大小                                                |
| OTA_MGR_EXTERN_FLASH             | OTA标识符管理分区分配到外部norflash                          |
| OTA_MGR_REGION_SIZE              | OTA标识符管理分区的大小                                      |
| OTA_MGR_DATA_ADDRESS             | OTA标识符管理分区的起始地址                                  |
| OTA_IMAGE_EXTERN_FLASH           | OTA镜像分区分配到外部norflash                                |
| OTA_IMAGE_ADDRESS                | OTA镜像分区的起始地址                                        |
| OTA_IMAGE_REGION_SIZE            | OTA镜像分区的大小                                            |
| CONFIG_LOG_UART_BAUDRATE         | 打印串口的波特率                                             |
| CONFIG_LOG_LEVEL                 | 日志打印等级。0，关闭；1，错误等级；2，告警等级；3，信息等级；4，调试等级。 |
| CONFIG_TEST                      | 测试模式。0，关闭；1，开启。                                 |

## 分区说明

分区如下：

- boot：引导系统，片内。负责引导进入app和对app的升级。

- app：应用程序，片内。负责执行用户的功能代码。
- ota_mgr：OTA标识符管理，可在片内也可在片外。负责管理OTA的功能，是否对app进行升级。
- ota_image：OTA镜像，可在片内也可在片外。暂存OTA镜像，可用于后续升级。

## 加密说明

启用加密功能后，OTA文件将被加密，从而提升安全性，防止文件被读取或反汇编。使用时，请确保 boot 工程和 app 工程中的密钥（key）和初始化向量（IV）保持一致，并在 app 工程中正确执行 `ota_generate.py` 脚本。

## 测试模式

测试模式主要用于在移植阶段测试接口是否可以正常工作。测试完成后，请立即关闭，否则会影响正常的功能使用。