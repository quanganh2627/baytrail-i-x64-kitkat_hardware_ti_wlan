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

#define LOG_TAG "wlan_prov"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/misc.h>
#include <cutils/android_reboot.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef BUILD_WITH_CHAABI_SUPPORT
#include "umip_access.h"
#endif

#define MAC_ADDRESS_LEN 6
const unsigned char NullMacAddr[MAC_ADDRESS_LEN] = { 0, 0, 0, 0, 0, 0 };

#define WIFI_PATH "/data/misc/wifi"
const char NVS_file_name[] = "/config/wifi/wl1271-nvs.bin";
const char Default_NVS_file_name[] = "/system/etc/wifi/wl1271-nvs.bin";
const char WLAN_SDIO_BUS_PATH[] = "/sys/bus/sdio/drivers/wl1271_sdio/";
#define WLAN_DRV_SDIO_NAME "wl1271_sdio"
#define SYSFS_SDIO_DEVICES_PATH "/sys/bus/sdio/devices/"
#define RFKILL_SYSFS_DEVICES_PATH "/sys/class/rfkill/"
#define NEW_NVS_FILE_NAME		WIFI_PATH"/new-nvs.bin"
#define TQS_FILE				"/etc/wifi/TQS.ini"
#define MAX_CALIBRATION_TRIES	3

/* pattern MAC address in NVS file */
#define NVS_LENGTH_TO_SET       0x01
#define NVS_LOWER_MAC_HIGHER    0x6D
#define NVS_LOWER_MAC_LOWER     0x54
#define NVS_HIGHER_MAC_HIGHER   0x71
#define NVS_HIGHER_MAC_LOWER    0x54
#define NVS_VALUE_TO_SET        0x00

const int debug = 0;

static int nvs_read_mac(unsigned char *MacAddr);
static int nvs_replace_mac(unsigned char *MacAddr);
static int wifi_calibration(void);

static char *find_entry_in_folder(char *folder, char *file)
{
	DIR *dir;
	struct dirent *entry;
	char *str = NULL;

	if (!folder || !file)
		return NULL;

	dir = opendir(folder);
	if (!dir)
		return NULL;

	do {
		entry = readdir(dir);
		if (!entry)
			break;
		if (!strcmp(entry->d_name, "."))
			continue;
		if (!strcmp(entry->d_name, ".."))
			continue;
		if (strcmp(entry->d_name, file))
			continue;

		str = (char *)malloc(strlen(entry->d_name) + 1);
		strncpy(str, entry->d_name, strlen(entry->d_name));
		break;
	} while (entry);

	closedir(dir);
	return str;
}

static char *parse_uevent_file(char *path, char *pattern)
{
	FILE *file;
	char line[128];
	char *result = NULL;

	if (!path || !pattern)
		goto out;

	file = fopen(path, "r");
	if (!file) {
		ALOGE("%s: %s file not found\n", __func__, path);
		goto out;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		if (!strncmp(pattern, line, strlen(pattern))) {
			/* Pattern found */
			result = (char *) malloc(strlen(line) -
						strlen(pattern) + 1);
			if (result) {
				strlcpy(result, line + strlen(pattern), strlen(line) - strlen(pattern) + 1);
			} else {
				ALOGE("%s: memory allocation failed\n", __func__);
			}
			goto close;
		}
	}
close:
	fclose(file);

	return result;
out:
	return NULL;
}

static int parse_type_file(char *path, char *pattern)
{
	FILE *file;
	char line[128];
	int found=0;
	if (!path || !pattern)
		goto out_error;

	file = fopen(path, "r");
	if (!file) {
		ALOGE("%s: %s file not found\n", __func__, path);
		goto out_error;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		if (!strncmp(pattern, line, strlen(pattern))) {
			/* Pattern found */
			found=1;
		}
	}

	fclose(file);
	if (found)
		return 1;

out_error:
	return 0;
}

static int get_wlan_rfkill_path(char *path, char* result_path, int sizeof_res)
{
	int ret = EXIT_FAILURE;
	struct dirent *entry;
	DIR *rfkill_path = opendir(path);
	char *type_entry = NULL;
	char sub_entry[256];
	if (!rfkill_path) {
		ALOGE("Cannot open directory, ret with %d\n", errno);
		ret = -EINVAL;
		goto exit;
	}

	do {
		entry = readdir(rfkill_path);
		if (!entry)
			break;
		if (!strcmp(entry->d_name, "."))
			continue;
		if (!strcmp(entry->d_name, ".."))
			continue;

		/* This folder shall contains only references to folders */
		snprintf(sub_entry, sizeof(sub_entry), RFKILL_SYSFS_DEVICES_PATH"%s/",entry->d_name);
		type_entry = find_entry_in_folder(sub_entry, "type");
		if (type_entry) {
			/* Read the content of type and check "DRIVER=" */
			char type_path[256];
			int found;
			strlcpy(type_path, sub_entry, sizeof(type_path));
			strlcat(type_path, "type", sizeof(type_path));
			/* Parse the file and get the value of type */
			found = parse_type_file(type_path, "wlan");
			if (!found) {
				free(type_entry);
				continue;
			} else {
				strlcpy(result_path, sub_entry, sizeof_res);
				strlcat(result_path, "state", sizeof_res);
				ret = EXIT_SUCCESS;
				free(type_entry);
				goto out;
			}
		}
	} while (entry);

out:
	closedir(rfkill_path);
exit:
	return ret;
}


static int toggle_wlan_radio(const char *rfkill_state_path, int enable) {
	FILE *file = NULL;
	int ret = EXIT_SUCCESS;

	file = fopen(rfkill_state_path, "w");
	if (!file) {
		ALOGE("Unable to open rfkill file %s in write mode", rfkill_state_path);
		ret = -EIO;
		goto out;
	}

	fprintf(file,"%d", enable);

	if (fclose(file)) {
		ALOGE("Unable to close file");
		ret = -EIO;
	}
	ALOGI("toggle_wlan_radio :%s %d", rfkill_state_path, enable);
out:
	return ret;
}


static int sdio_get_pci_id(char *sdio_device_path, char *pci_id, int sizeof_dev)
{
	int ret = EXIT_FAILURE;
	struct dirent *entry;
	DIR *sdio_path = opendir(sdio_device_path);
	char *uevent_entry = NULL;
	char sub_entry[256];
	if (!sdio_path) {
		ALOGE("Cannot open directory, ret with %d\n", errno);
		ret = -EINVAL;
		goto exit;
	}

	do {
		entry = readdir(sdio_path);
		if (!entry)
			break;
		if (!strcmp(entry->d_name, "."))
			continue;
		if (!strcmp(entry->d_name, ".."))
			continue;

		/* This folder shall contains only references to folders */
		snprintf(sub_entry, sizeof(sub_entry), SYSFS_SDIO_DEVICES_PATH"%s/",entry->d_name);
		uevent_entry = find_entry_in_folder(sub_entry, "uevent");
		if (uevent_entry) {
			/* Read the content of uevent and check "DRIVER=" */
			char uevent_path[256];
			char *result;
			strlcpy(uevent_path, sub_entry, sizeof(uevent_path));
			strlcat(uevent_path, "uevent", sizeof(uevent_path));
			/* Parse the file and get the value of DRIVER */
			result = parse_uevent_file(uevent_path, "DRIVER=");
			if (!result) {
				free(uevent_entry);
				continue;
			}

			if  (!strncmp(result, WLAN_DRV_SDIO_NAME,
				     strlen(WLAN_DRV_SDIO_NAME))) {
				strlcpy(pci_id, entry->d_name, sizeof_dev);
				free(result);
				free(uevent_entry);
				ret = EXIT_SUCCESS;
				goto out;
			} else {
				free(uevent_entry);
				free(result);
				continue;
			}
		}
	} while (entry);

out:
	closedir(sdio_path);
exit:
	return ret;
}

static int bind_unbind_driver(const char *driver_path, const char *driver_id,
							  const char *cmd) {
	FILE *file = NULL;
	int n = strlen(driver_path) + strlen(cmd);
	char *file_path;
	int ret = EXIT_SUCCESS;

	file_path = (char *) malloc(n+1);
	if (!file_path) {
		ALOGE("Not enough space\n");
		ret = -ENOMEM;
		goto fail;
	}

	snprintf(file_path, n+1, "%s%s", driver_path, cmd);

	file = fopen(file_path, "w");
	if (!file) {
		ALOGE("Unable to open file %s in write mode", file_path);
		ret = -EIO;
		goto open_fail;
	}

	fprintf(file,"%s", driver_id);

	if (fclose(file)) {
		ALOGE("Unable to close file");
		ret = -EIO;
	}

open_fail:
	free(file_path);
fail:
	return ret;
}

static inline int bind_wlan_sdio_drv(const char *sdio_bus_path,
	char *sdio_driver_id) {
	return bind_unbind_driver(sdio_bus_path, sdio_driver_id, "bind");
}

static inline int unbind_wlan_sdio_drv(const char *sdio_bus_path,
	char *sdio_driver_id) {
	return bind_unbind_driver(sdio_bus_path, sdio_driver_id, "unbind");
}

int main(int argc, char **argv)
{
	FILE *nvsBinFile = NULL;
	unsigned char NvsMacAddr[MAC_ADDRESS_LEN];
	unsigned char *ChaabiMacAddr = NULL;
	int res = 0;
	char device_id[64];
	char lrfkill_path[256];
	int unbind_bind_request = 0;

	/* Check parameters */
	if (argc != 1) {
		/* No param expected */
		return -1;
	}

       if (sdio_get_pci_id(SYSFS_SDIO_DEVICES_PATH, device_id, sizeof(device_id))) {
		ALOGE("no wlan device detected, exit...");
		return -1;
       }

#ifdef BUILD_WITH_CHAABI_SUPPORT
	/* Read MAC address from Chaabi */
	if (get_customer_data(ACD_WLAN_MAC_ADDR_FIELD_INDEX,
				(void ** const) &ChaabiMacAddr) != MAC_ADDRESS_LEN) {
#endif
		/* chaabi read error OR no chaabi support */
		ChaabiMacAddr = (unsigned char *) malloc(MAC_ADDRESS_LEN);
		if (ChaabiMacAddr)
			memcpy(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN);

		ALOGW("MAC not found");
#ifdef BUILD_WITH_CHAABI_SUPPORT
	}
#endif
	/* Check if calibration is requested (NVS file don't exist) */
	nvsBinFile = fopen(NVS_file_name, "rb");

	if (!nvsBinFile) {
		int nbCalibrationTries = 1;
		ALOGI("running calibration, try: %d",nbCalibrationTries);
		unbind_bind_request = 1;
		if (!get_wlan_rfkill_path(RFKILL_SYSFS_DEVICES_PATH, lrfkill_path, sizeof(lrfkill_path)))
			toggle_wlan_radio(lrfkill_path, 0);

		while (wifi_calibration()) {
			nbCalibrationTries++;
			if(nbCalibrationTries >= MAX_CALIBRATION_TRIES) {
				ALOGI("Rebooting after %d calibration tries", MAX_CALIBRATION_TRIES);
				goto fatal; //Reboot after 3 failed calibrations.
			}
			if (unbind_wlan_sdio_drv(WLAN_SDIO_BUS_PATH, device_id)
						|| bind_wlan_sdio_drv(WLAN_SDIO_BUS_PATH, device_id))
					{
						/*
						*Rebooting:  calibration failed, unbind/bind failed
						*/
						ALOGI("Rebooting: unbind/bind failed");
						goto fatal;
					}
			ALOGI("running calibration, try: %d",nbCalibrationTries);
		}
	} else {
		fclose(nvsBinFile);
		if (ChaabiMacAddr && (memcmp(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN) == 0)) {
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
	if (ChaabiMacAddr && (memcmp(ChaabiMacAddr, NullMacAddr, MAC_ADDRESS_LEN) == 0)) {
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
		ALOGI("MAC randomized");
	}

	/* Chaabi MAC @ shall be write in NVS file ? */
	if (ChaabiMacAddr && (memcmp(ChaabiMacAddr, NvsMacAddr, MAC_ADDRESS_LEN) != 0)) {
		if (nvs_replace_mac(ChaabiMacAddr)) {
			res =  -4;
			goto end;
		}
		ALOGI("MAC updated");
		unbind_bind_request = 1;
	}

end:
	/* Take into acount the new NVS firmware */
	sync();

	if (unbind_bind_request) {
		ALOGI("unbind/bind the driver");
		/*
		* If we are not allowed to bind/unbind the driver, the mac address as
		* well as the new configuration firmware (.nvs) will not be taken into
		* account. In that case, the reboot is required after flashing for the
		* time the board.
		*/
		if (unbind_wlan_sdio_drv(WLAN_SDIO_BUS_PATH, device_id)
			|| bind_wlan_sdio_drv(WLAN_SDIO_BUS_PATH, device_id)) {
			/*
			* Rebooting the board: In this level, the NVS was saved. The
			* next reboot will be fine since no need to bind/unbind the
			* driver.
			*/
			goto fatal;
		}
	}

	if(ChaabiMacAddr)
	    free(ChaabiMacAddr);

	return res;
fatal:
	sync();
	android_reboot(ANDROID_RB_RESTART, 0, 0);
	return res;
}



static int nvs_read_mac(unsigned char *MacAddr)
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

static int nvs_replace_mac(unsigned char *MacAddr)
{
	int err = 0;
	char system_cmd[500];

	snprintf(system_cmd, sizeof(system_cmd), "/system/bin/calibrator set nvs_mac"
						" %s %02x:%02x:%02x:%02x:%02x:%02x",
						NVS_file_name,
						MacAddr[0], MacAddr[1], MacAddr[2],
						MacAddr[3], MacAddr[4], MacAddr[5]);
	if (debug)
		ALOGI("cmd: %s",system_cmd);

	err = system(system_cmd);

	if (err)
		ALOGE("NVS update with new MAC error= %d",err);

	return err;
}

static int wifi_calibration(void)
{
	FILE *CuCmdFile = NULL;
	char system_cmd[500];
	int err = 0;
	int module_is_loaded = 0;

	/* start calibration & nvs update */
	snprintf(system_cmd, sizeof(system_cmd), "/system/bin/calibrator plt autocalibrate wlan0"
							" /lib/modules/wl12xx_sdio.ko %s %s %s",
							TQS_FILE,
							NVS_file_name,
							(unsigned char *)"08:00:28:DE:AD:00");
	if (debug)
		ALOGI("cmd: %s",system_cmd);

	err = system(system_cmd);

	if (err)
		ALOGE("Calibration error= %d",err);

	return err;
}


