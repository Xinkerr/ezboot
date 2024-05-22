/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_


#define FAL_PART_HAS_TABLE_CFG
#define FAL_USING_SFUD_PORT
#define RT_VER_NUM

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev ch32f2_onchip_flash;
extern struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &ch32f2_onchip_flash,                                           \
    &nor_flash0,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,       "boot",     "ch32_onchip",  0,          28*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "ota_mgr",  "ch32_onchip",  28*1024,    4*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "app",      "ch32_onchip",  32*1024,    96*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "FlashDB",  "norflash0",    0,          16*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "ota_image", "norflash0",   1024*1024,  1024*1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
