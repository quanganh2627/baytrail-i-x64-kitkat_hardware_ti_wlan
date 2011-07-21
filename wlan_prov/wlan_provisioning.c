/*
 * Copyright [2011] The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <android/log.h>

#define  PROGRAM_NAME "wlan_prov"

#ifdef BUILD_WITH_CHAABI_SUPPORT
#include "umip_access.h"
#endif

const char CU_CMD_file_name[] = "/data/calib_cmd";
const char CU_calibration_cmd[] = \
		"/ w p 1\n" \
		"/ w f 2\n" \
		"/ w l 2\n" \
		"/ t r h 0 7\n" \
		"/ t b b 375 128 0\n" \
		"/ t b t 1 0 0 0 0 0 0 0\n" \
		"/ q\n";

#define LOG_TAG "wlan_prov"

#define MAC_ADDRESS_LEN 6
const unsigned char NullMacAddr[MAC_ADDRESS_LEN] = { 0, 0, 0, 0, 0, 0 };

#define WIFI_PATH "/system/etc/wifi"

const char NVS_file_name[] = WIFI_PATH"/nvs_map.bin";

/* pattern MAC address in NVS file */
#define NVS_LENGTH_TO_SET       0x01
#define NVS_LOWER_MAC_HIGHER    0x6D
#define NVS_LOWER_MAC_LOWER     0x54
#define NVS_HIGHER_MAC_HIGHER   0x71
#define NVS_HIGHER_MAC_LOWER    0x54
#define NVS_VALUE_TO_SET        0x00

static int nvs_read_mac(unsigned char *MacAddr);
static int nvs_replace_mac(unsigned char *MacAddr);
static int wifi_calibration(void);

int main(int argc, char **argv)
{
	FILE *nvsBinFile = NULL;
	unsigned char NvsMacAddr[MAC_ADDRESS_LEN];
	unsigned char *ChaabiMacAddr = NULL;
	int res = 0;

	/* Check parameters */
	if (argc != 1) {
		/* No param expected */
		return -1;
	}

#ifdef BUILD_WITH_CHAABI_SUPPORT
	/* Read MAC address from Chaabi */
	if (get_customer_data(ACD_WLAN_MAC_ADDR_FIELD_INDEX,
				(void ** const) &ChaabiMacAddr) != MAC_ADDRESS_LEN) {
#endif
		/* chaabi read error OR no chaabi support */
		ChaabiMacAddr = (unsigned char *) malloc(MAC_ADDRESS_LEN);
		memcpy(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN);
#ifdef BUILD_WITH_CHAABI_SUPPORT
	}
#endif

	/* Check if calibration is requested (NVS file don't exist) */
	nvsBinFile = fopen(NVS_file_name, "rb");
	if (!nvsBinFile) {
		if (wifi_calibration()) {
			res =  -2;
			goto end;
		}
	} else {
		fclose(nvsBinFile);

		if (memcmp(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN) == 0) {
			/* NVS file already exist but chaabi read error */
			/* exit here to avoid randomize new MAC address */
			/* at each boot */
			res =  0;
			goto end;
		}
	}

	/* Read current MAC address from NVS file */
	if (nvs_read_mac(NvsMacAddr)) {
		res =  -3;
		goto end;
	}

	/* Shall randomize new MAC @ ? */
	if (memcmp(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN) == 0) {
		/* Chaabi MAC address is null due to engineering mode or
		 * chaabi read error so generate new MAC address randomly */
		struct timeval tv;

		gettimeofday(&tv, NULL);
		srand(tv.tv_usec);

		ChaabiMacAddr[0] = 0x08;
		ChaabiMacAddr[1] = 0x00;
		ChaabiMacAddr[2] = 0x28;
		ChaabiMacAddr[3] = (unsigned char) (rand() & 0xff);
		ChaabiMacAddr[4] = (unsigned char) (rand() & 0xff);
		ChaabiMacAddr[5] = (unsigned char) (rand() & 0xff);
	}

	/* Chaabi MAC @ shall be write in NVS file ? */
	if (memcmp(ChaabiMacAddr, NvsMacAddr, MAC_ADDRESS_LEN) != 0) {
		if (nvs_replace_mac(ChaabiMacAddr)) {
			res =  -4;
			goto end;
		}
	}

end:
	free(ChaabiMacAddr);

	return res;
}



int nvs_read_mac(unsigned char *MacAddr)
{
	FILE *nvsBinFile;
	unsigned char *pNvsContent = NULL;
	int res = 0;

	nvsBinFile = fopen(NVS_file_name, "rb");

	if (nvsBinFile) {
		pNvsContent = (void *) malloc(14);

		if (pNvsContent) {
			fread(pNvsContent, sizeof(char), 14, nvsBinFile);

			if ((pNvsContent[0] == NVS_LENGTH_TO_SET)
					 && (pNvsContent[1] == NVS_LOWER_MAC_HIGHER)
					 && (pNvsContent[2] == NVS_LOWER_MAC_LOWER)
					 && (pNvsContent[7] == NVS_LENGTH_TO_SET)
					 && (pNvsContent[8] == NVS_HIGHER_MAC_HIGHER)
					 && (pNvsContent[9] == NVS_HIGHER_MAC_LOWER)
					 && (pNvsContent[12] == NVS_VALUE_TO_SET)
					 && (pNvsContent[13] == NVS_VALUE_TO_SET)) {
				MacAddr[5] = pNvsContent[3];
				MacAddr[4] = pNvsContent[4];
				MacAddr[3] = pNvsContent[5];
				MacAddr[2] = pNvsContent[6];

				MacAddr[1] = pNvsContent[10];
				MacAddr[0] = pNvsContent[11];
			} else
				res = -1;

			free(pNvsContent);
		}
		fclose(nvsBinFile);
	} else
		res = -2;

	return res;
}

int nvs_replace_mac(unsigned char *MacAddr)
{
	int res = 0;
	FILE *nvsBinFile;
	int fsize;
	unsigned char *pNvsContent = NULL;

	/* Read whole NVS file before modifying file */
	nvsBinFile = fopen(NVS_file_name, "r+b");
	if (nvsBinFile) {
		fseek(nvsBinFile, 0L, SEEK_END);
		fsize = ftell(nvsBinFile);

		pNvsContent = (unsigned char *) malloc(fsize);
		if (pNvsContent) {
			fseek(nvsBinFile, 0L, SEEK_SET);
			fread(pNvsContent, sizeof(char), fsize, nvsBinFile);

			/* check MAC @ pattern before updating file */
			if ((pNvsContent[0] == NVS_LENGTH_TO_SET)
					 && (pNvsContent[1] == NVS_LOWER_MAC_HIGHER)
					 && (pNvsContent[2] == NVS_LOWER_MAC_LOWER)
					 && (pNvsContent[7] == NVS_LENGTH_TO_SET)
					 && (pNvsContent[8] == NVS_HIGHER_MAC_HIGHER)
					 && (pNvsContent[9] == NVS_HIGHER_MAC_LOWER)
					 && (pNvsContent[12] == NVS_VALUE_TO_SET)
					 && (pNvsContent[13] == NVS_VALUE_TO_SET)) {
				pNvsContent[3] = MacAddr[5];
				pNvsContent[4] = MacAddr[4];
				pNvsContent[5] = MacAddr[3];
				pNvsContent[6] = MacAddr[2];

				pNvsContent[10] = MacAddr[1];
				pNvsContent[11] = MacAddr[0];

				fseek(nvsBinFile, 0L, SEEK_SET);

				fwrite((void *)pNvsContent, 1L, fsize, nvsBinFile);
			} else
				res = -1;

			free(pNvsContent);
		} else
			res = -2;

		fclose(nvsBinFile);
	} else
		res = -3;

	return res;
}

int wifi_calibration(void)
{
	FILE *CuCmdFile = NULL;
	char system_cmd[200];
	int err = 0;
	int err_end = 0;
	int module_is_loaded = 0;

	/* generate wlan_cu command for calibration */
	CuCmdFile = fopen(CU_CMD_file_name, "w");
	if (CuCmdFile) {
		fwrite(CU_calibration_cmd, 1, sizeof(CU_calibration_cmd), CuCmdFile);
		fclose(CuCmdFile);
	} else
		return -1;

	/* Loading WLAN driver module */
	err = system("/system/bin/insmod /lib/modules/tiwlan_drv.ko");
	if (!err) {
		module_is_loaded = 1;

		/* Loading WLAN FW ... */
		err = system("/system/bin/wlan_loader -i "WIFI_PATH"/tiwlan.ini -f "WIFI_PATH"/firmware.bin");
		if (err) {
			__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
			"WLAN_PROV: can't load TIWLAN firmware (%d)", err);
			goto end;
		}

		/* Start calibration */
		sprintf(system_cmd, "/system/bin/wlan_cu -b < %s", CU_CMD_file_name);
		err = system(system_cmd);
		if (err) {
			__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
				"WLAN_PROV: calibration fails (%d)", err);
			goto end;
		}
	} else {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
			"WLAN_PROV: can't load tiwlan_drv.ko (%d)", err);
	}

end:
	/* unload WLAN driver module if necessary */
	if (module_is_loaded) {
		err_end = system("/system/bin/rmmod tiwlan_drv.ko");
		if (err_end) {
			__android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
				"WLAN_PROV: tiwlan_drv.ko  module removing error (%d)", err);

			/* return local error if no previous error */
			err = !err ? err_end : err;
		}
	}

	/* removing temporary file */
	err_end = unlink(CU_CMD_file_name);
	/* return local error if no previous error */
	err = !err ? err_end : err;

	return err;
}

