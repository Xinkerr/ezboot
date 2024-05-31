#include <stdio.h>
#include <stdint.h>
#include <boot.h>
#include <mlog.h>
#include <ota.h>
#include <ota_mgr.h>
#if OTA_MGR_EXTERN_FLASH | OTA_IMAGE_EXTERN_FLASH
#include  <norflash.h>
#endif


int main( void )
{
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
		
		default:
			break;
		}	
	}
	
	mlog( "__________________________________________\r\n" );
	app_enter();
}

