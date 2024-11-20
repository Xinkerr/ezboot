/**
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

#include <stdio.h>
#include <stdint.h>
#include <boot.h>
#include <mlog.h>
#include <ota.h>
#include <ota_mgr.h>
#if OTA_MGR_EXTERN_FLASH | OTA_IMAGE_EXTERN_FLASH
#include  <norflash.h>
#endif

int __weak system_init(void)
{
	return 0;
}


int main( void )
{
	system_init();

    mlog_init();
	
	mlog("\r\n");
	mlog(" ______ __________   ____   ____ _______\r\n");
	mlog("|  ____|___  /  _ \\ / __ \\ / __ \\__   __|\r\n");
	mlog("| |__     / /| |_) | |  | | |  | | | |   \r\n");
	mlog("|  __|   / / |  _ <| |  | | |  | | | |   \r\n");
	mlog("| |____ / /__| |_) | |__| | |__| | | |   \r\n");
	mlog("|______/_____|____/ \\____/ \\____/  |_|   \r\n");
	mlog("Version: 0.1\r\n");
	mlog("Build: %s\r\n", __DATE__);
	
	#if OTA_MGR_EXTERN_FLASH | OTA_IMAGE_EXTERN_FLASH
	norflash_init();
	#endif

	#if CONFIG_TEST
		#if OTA_MGR_EXTERN_FLASH | OTA_IMAGE_EXTERN_FLASH
		norflash_test();
		#endif
		ota_mgr_test();
		ota_flash_test();
	#endif

	ota_mgr_state_t state = ota_mgr_state_get();
	mlog_i("ota state:%d", state);

	if(state == OTA_REQUEST)
	{
		int ret = ota_firmware_update();
		switch (ret)
		{
		case OTA_SUCCESS:
			ota_mgr_state_set(OTA_WRITE_COMPLETED);
			ota_image_erase();
			break;
		
		case OTA_FLASH_ERR:
			ota_mgr_state_set(OTA_STA_WR_ERROR);
			break;

		case OTA_IMAGE_CHECK_ERR:
			ota_mgr_state_set(OTA_IMAGE_CHECKSUM_ERR);
			break;

		case OTA_IMAGE_SIZE_ERR:
			ota_mgr_state_set(OTA_FW_OVER_SIZE);
			break;

		case OTA_IMAGE_ENCRYPT_OVERFLOW:
			ota_mgr_state_set(OTA_ENCRYPT_DATA_OVERFLOW);
			break;
		
		default:
			break;
		}	
	}
	
	mlog( "__________________________________________\r\n" );
	app_enter();
}

