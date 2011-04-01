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

#include <asm/arch/hardware.h>

#ifndef OMAP_CTRL_BASE
#define OMAP_CTRL_BASE          		OMAP343X_CTRL_BASE
#endif
#define OMAP_HSMMC2_BASE				0x480b4000
#define CONTROL_REG32_34XX(offset)		__REG32(OMAP_CTRL_BASE + (offset))
#define OMAP2_CONTROL_DEVCONF1			CONTROL_REG32_34XX(0x2D8)
#define CONTROL_PADCONF_SYS_BOOT1       CONTROL_REG32_34XX(0xA0C) /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       CONTROL_REG32_34XX(0xC8)  /* WLAN_IRQ    */
#define CONTROL_PADCONF_MMC2_CLK		CONTROL_REG32_34XX(0x158)  /* mmc2_cmd */
#define CONTROL_PADCONF_MMC2_DAT0		CONTROL_REG32_34XX(0x15C)  /* mmc2_dat0, mmc2_dat1 */
#define CONTROL_PADCONF_MMC2_DAT2		CONTROL_REG32_34XX(0x160)  /* mmc2_dat2, mmc2_dat3 */
#define PMENA_GPIO                      3
#define IRQ_GPIO                        61
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
