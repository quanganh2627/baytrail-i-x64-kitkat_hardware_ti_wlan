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

#define CONTROL_PADCONF_UART1_TX_OFFSET 0xB3
#define CONTROL_PADCONF_UART1_RX_OFFSET 0xB4
#define CONTROL_MMC2_CMD_PAD_PA         0xFB
#define CONTROL_MMC2_DAT3_PAD_PA        0xFA
#define CONTROL_MMC2_DAT2_PAD_PA        0xFD
#define CONTROL_MMC2_DAT1_PAD_PA        0xFE
#define CONTROL_MMC2_DAT0_PAD_PA        0xFC

#define OMAP24XX_VA_SYSTEM_CONTROL_BASE	IO_ADDRESS(OMAP24XX_SYSTEM_CONTROL_BASE)
#define GPIO_9                          9
#define GPIO_10                         10
#define GPIO_OUTPUT		                0
#define MUX_MODE_0                      0
#define MUX_MODE_3                      3

#define IRQ_GPIO                        GPIO_9
#define IRQ_GPIO_MCS8                   GPIO_9  /* for zoom2 compatibility */

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
