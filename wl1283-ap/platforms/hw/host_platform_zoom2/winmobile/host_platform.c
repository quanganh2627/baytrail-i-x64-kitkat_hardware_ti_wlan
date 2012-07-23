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

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <gpio.h>
#include <omap3430_config.h>
#include <omap3430_base_regs.h>
#include "tidef.h"
#include "host_platform.h"

#define OS_API_MEM_ADRR  	0x0000000
#define OS_API_REG_ADRR  	0x300000
#define SDIO_ATTEMPT_LONGER_DELAY_WM  400

/* Enable GPIO Power line */
typedef struct {
	int GPIO_POWER_LINE;
} GPIO_PWR;
GPIO_PWR gpio_pwr;

void Get_GPIOPWR_LINE(int POWER_LINE)
{
	gpio_pwr.GPIO_POWER_LINE = POWER_LINE;
}
static int OMAP3430_TNETW_Power(int power_on)
{
    HANDLE hGPIO = GPIOOpen();

   /* Power enable */
 
   if(hGPIO == NULL) 
   {
        printf("OMAP3430_TNETW_Power() GPIOOpen FAILED\n");
        return ERROR_1;
   }
    
    GPIOSetMode(hGPIO, gpio_pwr.GPIO_POWER_LINE, GPIO_DIR_OUTPUT);
    if (power_on)
    {
        GPIOSetBit(hGPIO, gpio_pwr.GPIO_POWER_LINE);
    }
    else
    {
        GPIOClrBit(hGPIO, gpio_pwr.GPIO_POWER_LINE);
    }
    GPIOClose(hGPIO);

    return SUCCESS;
    
} /* OMAP3430_TNETW_Power() */

/*-----------------------------------------------------------------------------

Routine Name:

        hPlatform_hardResetTnetw

Routine Description:

        set the GPIO to low after awaking the TNET from ELP.

Arguments:

        OsContext - our adapter context.


Return Value:

        None

-----------------------------------------------------------------------------*/
int hPlatform_hardResetTnetw( void )
{
    int err;

      /* Turn power OFF*/
    if ((err = OMAP3430_TNETW_Power(0)) == 0)
    {
        Sleep(200);
        /* Turn power ON*/
        err = OMAP3430_TNETW_Power(1);
        Sleep(200);
    }
    return err;

} /* hPlatform_hardResetTnetw() */

/*--------------------------------------------------------------------------------------*/

/* Turn device power off */
int hPlatform_DevicePowerOff (void)
{
    int err;
    
    err = OMAP3430_TNETW_Power(0);
    
    Sleep(10);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power off according to a given delay */
int hPlatform_DevicePowerOffSetLongerDelay (void)
{
    int err;
    
    err = OMAP3430_TNETW_Power(0);
    
    Sleep(SDIO_ATTEMPT_LONGER_DELAY_WM);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power on */
int hPlatform_DevicePowerOn (void)
{
    int err;

    err = OMAP3430_TNETW_Power(1);

    /* Should not be changed, 50 msec cause failures */
    Sleep(70);

    return err;
}

/*--------------------------------------------------------------------------------------*/

int hPlatform_Wlan_Hardware_Init(void)
{
    return SUCCESS;

} /* hPlatform_Wlan_Hardware_Init() */

void hPlatform_Wlan_Hardware_DeInit(void)
{

}

/*--------------------------------------------------------------------------------------*/

void hPlatform_freeInterrupt(void) 
{

    printf("%s *NOT* implemented !!!!!!!!!!!!!!!\n",__FUNCTION__);
}

/****************************************************************************************
 *                        hPlatform_hwGetRegistersAddr()                                 
 ****************************************************************************************
DESCRIPTION:	

ARGUMENTS:		

RETURN:			

NOTES:         	
*****************************************************************************************/
void*
hPlatform_hwGetRegistersAddr(
        TI_HANDLE OsContext
        )
{
	return (void*)OS_API_REG_ADRR;
}

/****************************************************************************************
 *                        hPlatform_hwGetMemoryAddr()                                 
 ****************************************************************************************
DESCRIPTION:	

ARGUMENTS:		

RETURN:			

NOTES:         	
*****************************************************************************************/
void*
hPlatform_hwGetMemoryAddr(
        TI_HANDLE OsContext
        )
{
	return (void*)OS_API_MEM_ADRR;
}


