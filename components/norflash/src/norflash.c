#include <norflash_spi.h>
#include <norflash.h>
#include <mlog.h>

#define CMD_READ_DATA                               0x03
#define CMD_WRITE_DISABLE                           0x04
#define CMD_READ_STATUS_REGISTER                    0x05
#define CMD_WRITE_ENABLE                            0x06
#define CMD_MANUFACTURER_DEVICE_ID                  0x90

#define CMD_PAGE_PROGRAM                            0x02
#define CMD_SECTOR_ERASE                            0x20
#define ERASE_SIZE                                  4096
#define PAGE_SIZE                                   256

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

int norflash_init(void)
{
    norflash_spi_init();
    uint16_t norflash_id = norflash_read_ID();
    mlog_hex_i("norflash ID: ", &norflash_id, sizeof(norflash_id));
    return 0;
}

int norflash_erase(uint32_t addr, uint32_t size)
{	
    uint32_t i;
    uint32_t erase_addr;
    erase_addr = addr;
    norflash_write_enable();                  
    norflash_wait_busy();   
  	norflash_spi_cs(true);                         
    for(i=0; i<size; i+=ERASE_SIZE)
    {
        addr += i;
        norflash_spi_transfer(CMD_SECTOR_ERASE);      	//发送扇区擦除指令 
        norflash_spi_transfer((uint8_t)((erase_addr)>>16));  
        norflash_spi_transfer((uint8_t)((erase_addr)>>8));   
        norflash_spi_transfer((uint8_t)erase_addr);  
    }
	norflash_spi_cs(false);                  	      
    norflash_wait_busy();   				   		    //等待擦除完成
    norflash_write_disable();
    return 0;
}

int norflash_read(uint32_t addr, void* buf, uint32_t size)
{
    uint32_t i;			
    uint8_t* read_buf = (uint8_t*)buf;					    
	norflash_spi_cs(true);
    norflash_spi_transfer(CMD_READ_DATA);         	//发送读取命令   
    norflash_spi_transfer((uint8_t)((addr)>>16));  
    norflash_spi_transfer((uint8_t)((addr)>>8));   
    norflash_spi_transfer((uint8_t)addr);   
    for(i=0; i<size; i++)
	{ 
        read_buf[i]=norflash_spi_transfer(0XFF);   	//循环读数  
    }
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

int norflash_write(uint32_t addr, const void* buf, uint32_t size)
{
    uint32_t page_remain;
    uint8_t* write_buf = (uint8_t*)buf;	 
    if(size < PAGE_SIZE)  
        page_remain = size;//不大于PAGE_SIZE
    else
	    page_remain = PAGE_SIZE - addr % PAGE_SIZE; //单页剩余的字节数		 	    
        
	while(page_remain)
	{	   
		norflash_write_page(addr, write_buf, page_remain);
        write_buf += page_remain;
		addr += page_remain;
        size -= page_remain;	    //减去已经写入了的字节数
        if(size > PAGE_SIZE)
            page_remain = PAGE_SIZE; //一次可以写入一页数据
        else 
            page_remain = size;
	};	
    return 0;
}

