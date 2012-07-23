/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/*--------------------------------------------------------------------------
 Module:      host_platform_sdio.h

 Purpose:     This module defines unified interface to the host platform specific
              sources and services.

--------------------------------------------------------------------------*/

#ifndef __HOST_PLATFORM_SDIO__H__
#define __HOST_PLATFORM_SDIO__H__

#include <mach/hardware.h>

#define OMAP_CTRL_BASE                          OMAP343X_CTRL_BASE
#define OMAP_HSMMC2_BASE                                0x480b4000

#define CONTROL_PADCONF_MMC2_CLK        0x48002158   /* mmc2_cmd */
#define CONTROL_PADCONF_MMC2_DAT0       0x4800215C   /* mmc2_dat0, mmc2_dat1 */
#define CONTROL_PADCONF_MMC2_DAT2       0x48002160   /* mmc2_dat2, mmc2_dat3 */

#ifdef EXPANSION_BOARD_MCS7
#define CONTROL_PADCONF_SYS_BOOT1       0x48002164   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x48002168   /* WLAN_IRQ    */
#define PMENA_GPIO                      136
#define IRQ_GPIO                        139
#else 
#ifdef EXPANSION_BOARD_TST
#define CONTROL_PADCONF_SYS_BOOT1       0x48002A0C   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x480020C8   /* WLAN_IRQ    */
#define PMENA_GPIO                      3
#define IRQ_GPIO                        61
#else
#define CONTROL_PADCONF_GPMC_NCS5       0x480020B8   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x480020C8   /* WLAN_IRQ    */
#define PMENA_GPIO                      57
#define IRQ_GPIO                        61
#endif
#endif

#define MUXMODE_3                       3
#define TNETW_IRQ                       (OMAP_GPIO_IRQ(IRQ_GPIO))
#define TIWLAN_IRQ_POLL_INTERVAL	    HZ/100
#define HZ_IN_MSEC						HZ/1000
#define TIWLAN_IRQ_POLL_INTERVAL_MS		TIWLAN_IRQ_POLL_INTERVAL/HZ_IN_MSEC

int 
hPlatform_initInterrupt(
	void* tnet_drv,
	void* handle_add
	);

void*
hPlatform_hwGetRegistersAddr(
    TI_HANDLE OsContext
    );

void*
hPlatform_hwGetMemoryAddr(
    TI_HANDLE OsContext
    );

void hPlatform_freeInterrupt(void);

int  hPlatform_hardResetTnetw(void);
int  hPlatform_Wlan_Hardware_Init(void);
void hPlatform_Wlan_Hardware_DeInit(void);
int  hPlatform_DevicePowerOff(void);
int  hPlatform_DevicePowerOffSetLongerDelay(void);
int  hPlatform_DevicePowerOn(void);
#endif /* __HOST_PLATFORM_SDIO__H__ */
