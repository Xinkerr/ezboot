#include <norflash_spi.h>
#include <norflash.h>

#define CMD_PAGE_PROGRAM                            0x02
#define CMD_READ_DATA                               0x03
#define CMD_WRITE_DISABLE                           0x04
#define CMD_READ_STATUS_REGISTER                    0x05
#define CMD_WRITE_ENABLE                            0x06
#define CMD_MANUFACTURER_DEVICE_ID                  0x90

#ifdef CONFIG_NORFLASH_CMD_SECTOR_ERASE
#define CMD_SECTOR_ERASE                            CONFIG_NORFLASH_CMD_SECTOR_ERASE
#else
#define CMD_SECTOR_ERASE                            0x20
#endif

#ifdef CONFIG_NORFLASH_ERASE_SIZE
#define ERASE_SIZE                                  CONFIG_NORFLASH_ERASE_SIZE
#else
#define ERASE_SIZE                                  4096
#endif

#ifdef CONFIG_NORFLASH_PAGE_SIZE
#define PAGE_SIZE                                   CONFIG_NORFLASH_PAGE_SIZE
#else
#define PAGE_SIZE                                   256
#endif

#ifdef CONFIG_LOG_LEVEL
#include #include <mlog.h>
#define nf_log_hex_i    mlog_hex_i
#define nf_log_i        mlog_i
#else
#define nf_log_hex_i(...)
#define nf_log_i(...)
#endif

static void norflash_write_enable(void)   
{
	norflash_spi_cs(true);                            
    norflash_spi_transfer(CMD_WRITE_ENABLE); 	//发送写使能  
	norflash_spi_cs(false);	                           	 	      
} 

static void norflash_write_disable(void)   
{  
	norflash_spi_cs(true);                         
    norflash_spi_transfer(CMD_WRITE_DISABLE);  //发送写禁止指令    
	norflash_spi_cs(false);                           	      
} 

static uint8_t norflash_read_SR(void)   
{  
	uint8_t byte=0;   
	norflash_spi_cs(true);                           
	norflash_spi_transfer(CMD_READ_STATUS_REGISTER); //发送读取状态寄存器命令    
	byte = norflash_spi_transfer(0Xff);         
	norflash_spi_cs(false);                          
	return byte;   
}

static void norflash_wait_busy(void)   
{   
	while((norflash_read_SR() & 0x01) == 0x01);  // 等待BUSY位清空
}  

uint16_t norflash_read_ID(void)
{
	uint16_t temp = 0;	  
    norflash_spi_cs(true);			    
	norflash_spi_transfer(CMD_MANUFACTURER_DEVICE_ID);	    
	norflash_spi_transfer(0x00); 	    
	norflash_spi_transfer(0x00); 	    
	norflash_spi_transfer(0x00); 	 			   
	temp |= norflash_spi_transfer(0xFF) << 8;  
	temp |= norflash_spi_transfer(0xFF);	 
    norflash_spi_cs(false);					    
	return temp;
}   

/**
 * @brief 初始化NORFLASH设备
 * 
 * 本函数负责初始化SPI总线并验证NORFLASH设备的存在。它首先初始化用于与NORFLASH通信的SPI总线，
 * 然后读取NORFLASH的设备ID，并记录该ID以供调试使用。
 * 
 * @return 返回0表示初始化成功；返回-1表示SPI总线初始化失败；返回-2表示无法读取到有效的NORFLASH设备ID。
 */
int norflash_init(void)
{
    int ret;
    uint16_t norflash_id = 0;

    // 初始化SPI总线以用于NOR闪存通信
    ret = norflash_spi_init();
    if(ret != 0)
        return -1;
    
    // 读取NOR闪存的设备ID
    norflash_id = norflash_read_ID();
    if(norflash_id == 0)
        return -2;
    
    // 记录NOR闪存设备ID的日志，以16进制输出
    nf_log_hex_i("norflash ID: ", &norflash_id, sizeof(norflash_id));
    
    return 0; // 初始化成功
}

/**
 * @brief 擦除NORFLASH指定区域
 * 
 * @param addr 擦除操作的起始地址
 * @param size 擦除的大小，以字节为单位
 * @return int 操作成功返回0，失败返回非0值
 */
int norflash_erase(uint32_t addr, uint32_t size)
{
    uint32_t i;
    uint32_t erase_addr;
    erase_addr = addr;
    
    /* 启用flash写操作 */
    norflash_write_enable();                   
    /* 等待flash准备就绪 */
    norflash_wait_busy();   

    /* 选中SPI Flash设备 */
    norflash_spi_cs(true);                         
    /* 遍历需要擦除的区域 */
    for(i=0; i<size; i+=ERASE_SIZE)
    {
        addr += i;
        /* 发送扇区擦除指令 */
        norflash_spi_transfer(CMD_SECTOR_ERASE);       
        norflash_spi_transfer((uint8_t)((erase_addr)>>16));  
        norflash_spi_transfer((uint8_t)((erase_addr)>>8));   
        norflash_spi_transfer((uint8_t)erase_addr);  
    }
    /* 取消SPI Flash设备选中 */
    norflash_spi_cs(false);                  	      
    /* 等待擦除操作完成 */
    norflash_wait_busy();   				   		   	
    /* 禁用flash写操作 */
    norflash_write_disable();
    return 0;
}

/**
 * @brief 从NORFLASH中读取数据。
 * 
 * @param addr 要读取数据的起始地址。
 * @param buf 用于存储读取数据的缓冲区指针。
 * @param size 要读取的数据大小，以字节为单位。
 * @return 0，表示成功。
 */
int norflash_read(uint32_t addr, void* buf, uint32_t size)
{
    uint32_t i;			
    uint8_t* read_buf = (uint8_t*)buf;					    
    // 选中SPI Flash设备
    norflash_spi_cs(true);
    // 发送读取命令
    norflash_spi_transfer(CMD_READ_DATA);         	      
    // 发送地址的高16位
    norflash_spi_transfer((uint8_t)((addr)>>16));  
    // 发送地址的高8位
    norflash_spi_transfer((uint8_t)((addr)>>8));   
    // 发送地址的低8位
    norflash_spi_transfer((uint8_t)addr);   
    // 循环读取数据
    for(i=0; i<size; i++)
	{ 
        read_buf[i]=norflash_spi_transfer(0XFF);   	
    }
    // 解除SPI Flash设备选中
    norflash_spi_cs(false);	
    return 0;
}

static void norflash_write_page(uint32_t addr, const uint8_t* buf, uint32_t size)
{
 	uint32_t i;
    norflash_write_enable();                  
	norflash_spi_cs(true);                           
    norflash_spi_transfer(CMD_PAGE_PROGRAM);      	//发送写页命令   
    norflash_spi_transfer((uint8_t)((addr)>>16));  
    norflash_spi_transfer((uint8_t)((addr)>>8));   
    norflash_spi_transfer((uint8_t)addr);   
    for(i=0; i<size; i++)
    {
        norflash_spi_transfer(buf[i]);
    }
	norflash_spi_cs(false);                           
	norflash_wait_busy();					   		
    norflash_write_disable();
} 

/**
 * @brief 将数据写入NORFLASH。
 *
 * 此函数将缓冲区中的数据以页为单位写入NORFLASH。如果数据大小不是页大小的倍数，
 * 则会在最后一个页中写入剩余数据。
 *
 * @param addr 要开始写入的NORFLASH地址。
 * @param buf 待写入数据的缓冲区指针。
 * @param size 从缓冲区中要写入的数据字节数。
 * @return int 0，表示成功。
 */
int norflash_write(uint32_t addr, const void* buf, uint32_t size)
{
    uint32_t page_remain;
    uint8_t* write_buf = (uint8_t*)buf;	 
    
    // 根据起始地址是否在页内确定第一个页要写入的字节数。
    if(size < PAGE_SIZE)  
        page_remain = size; // 大小小于一页，因此写入整个大小。
    else
	    page_remain = PAGE_SIZE - addr % PAGE_SIZE; // 计算起始页中的剩余字节数。 

    // 循环直到所有数据被写入。
    while(page_remain)
    {
        // 将数据写入当前页。
        norflash_write_page(addr, write_buf, page_remain);
        write_buf += page_remain; // 移动缓冲区指针到下一个要写入的段。
        addr += page_remain; // 移动地址到下一页。
        size -= page_remain; // 从总大小中减去已写入的字节数。
        
        // 为下一次迭代计算剩余要写入的字节数。
        // 如果还有超过一页的字节，将page_remain设置为页大小，
        // 否则，将其设置为剩余大小。
        if(size > PAGE_SIZE)
            page_remain = PAGE_SIZE;
        else 
            page_remain = size;
    };	
    return 0; // 表示成功。
}

