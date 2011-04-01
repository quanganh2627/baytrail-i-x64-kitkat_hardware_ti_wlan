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
#include "tidef.h"
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/tc.h>
#include <linux/delay.h>

#include "host_platform.h"
#include "ioctl_init.h"
#include "WlanDrvIf.h"

#include "Device1273.h"


#define OS_API_MEM_ADRR  	0x0000000
#define OS_API_REG_ADRR  	0x300000

#define SDIO_ATTEMPT_LONGER_DELAY_LINUX  150

/*--------------------------------------------------------------------------------------*/

#if 0
static void pad_config(unsigned long pad_addr, u32 andmask, u32 ormask)
{
	int val;
	u32 *addr;

	addr = (u32 *) ioremap(pad_addr, 4);
	if (!addr) {
		printk(KERN_ERR "OMAP3430_pad_config: ioremap failed with addr %lx\n", pad_addr);
		return;
	}

	val =  __raw_readl(addr);
	val &= andmask;
	val |= ormask;
	__raw_writel(val, addr);

	iounmap(addr);
}
#endif

static int OMAP3430_TNETW_Power(int power_on)
{
	if (power_on) {
		omap_set_gpio_dataout(PMENA_GPIO, 1);
	} else {
		omap_set_gpio_dataout(PMENA_GPIO, 0);
	}

	return 0;    
}

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

int hPlatform_hardResetTnetw(void)
{
  int err;

    /* Turn power OFF*/
  if ((err = OMAP3430_TNETW_Power(0)) == 0)
  {
    mdelay(500);
    /* Turn power ON*/
    err = OMAP3430_TNETW_Power(1);
    mdelay(50);
  }
  return err;

} /* hPlatform_hardResetTnetw() */

/* Turn device power off */
int hPlatform_DevicePowerOff (void)
{
    int err;
    
    err = OMAP3430_TNETW_Power(0);
    
    mdelay(10);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power off according to a given delay */
int hPlatform_DevicePowerOffSetLongerDelay(void)
{
    int err;
    
    err = OMAP3430_TNETW_Power(0);
    
    mdelay(SDIO_ATTEMPT_LONGER_DELAY_LINUX);
    
    return err;
}

/* Turn device power on */
int hPlatform_DevicePowerOn (void)
{
    int err;

    err = OMAP3430_TNETW_Power(1);

    /* Should not be changed, 50 msec cause failures */
    mdelay(70);

    return err;
}

/*--------------------------------------------------------------------------------------*/

int hPlatform_Wlan_Hardware_Init(void)
{
        int val=0;

#ifdef EXPANSION_BOARD_MCS7
        printk("hPlatform_Wlan_Hardware_Init: Select WLAN_EN = GPIO_136, WLAN_IRQ = GPIO_139\n");
        /* choose gpio 136, pull up, */
        /* Setting MUX Mode 4 , Pull bits  */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx0 0100 */

        val= omap_readl(CONTROL_PADCONF_SYS_BOOT1);
        val &=0xFFFFFFE0;
        val |=0x1C;
        omap_writel( val, CONTROL_PADCONF_SYS_BOOT1 );

        /* choose gpio 139, pull up, activated */
        /* Setting MUX Mode 4 , Pull bits 3 */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx1 1100 */
        val= omap_readl(CONTROL_PADCONF_GPMC_NBE1);
        val &=0xFFE0FFFF;
        val |=0x001C0000;
        omap_writel( val,CONTROL_PADCONF_GPMC_NBE1 );
#else
#ifdef EXPANSION_BOARD_TST
        printk("hPlatform_Wlan_Hardware_Init: Select WLAN_EN = GPIO_3, WLAN_IRQ = GPIO_61\n");
        /* choose gpio 3, pull down, not activated */
        /* Setting MUX Mode 4 , Pull bits 0 */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx0 0100 */

        val= omap_readl(CONTROL_PADCONF_SYS_BOOT1);
        val &=0xFFFFFFE4;
        val |=0x4;
        omap_writel( val, CONTROL_PADCONF_SYS_BOOT1 );

        /* choose gpio 61, pull up, activated */
        /* Setting MUX Mode 4 , Pull bits 3 */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx1 1100 */
        val= omap_readl(CONTROL_PADCONF_GPMC_NBE1);
        val &=0xFFFFFFFC;
        val |=0x0000001C;
        omap_writel( val,CONTROL_PADCONF_GPMC_NBE1 );
#else
        printk("hPlatform_Wlan_Hardware_Init: Select WLAN_EN = GPIO_57, WLAN_IRQ = GPIO_61\n");
        /* choose gpio 57, pull down, not activated */
        /* Setting MUX Mode 4 , Pull bits 0 */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx0 0100 */

        val= omap_readl(CONTROL_PADCONF_GPMC_NCS5);
        val &=0xFFE0FFFF;
        val |=0x001C0000;
        omap_writel( val, CONTROL_PADCONF_GPMC_NCS5 );

        /* choose gpio 61, pull up, activated */
        /* Setting MUX Mode 4 , Pull bits 3 */
        /* Should set (x is don't change):  xxxx xxxx xxxx xxxx xxxx xxxx xxx1 1100 */
        val= omap_readl(CONTROL_PADCONF_GPMC_NBE1);
        val &=0xFFFFFFFC;
        val |=0x0000001C;
        omap_writel( val,CONTROL_PADCONF_GPMC_NBE1 );
#endif
#endif

        /*
          * set pull up on all SDIO lines from the 3430 registers
          * Setting MUX Mode of 0, and pull bits to 3
          */

        /* set for mmc2_cmd - second half of the padconf register
          * Should set (x is don't change):  xxxx xxxx xxx1 1000 xxxx xxxx xxx1 1000 */

        val= omap_readl(CONTROL_PADCONF_MMC2_CLK);
        val &=0xFFE0FFE0;
        val |=0x00180018 ;
        omap_writel( val, CONTROL_PADCONF_MMC2_CLK );


        /* set for mmc2_dat0 and dat1 - both parts of the padconf register
          * Should set (x is don't change):  xxxx xxxx xxx1 1000 xxxx xxxx xxx1 1000 */

        val= omap_readl(CONTROL_PADCONF_MMC2_DAT0);
        val &=0xFFE0FFE0;
        val |=0x00180018 ;
        omap_writel( val,CONTROL_PADCONF_MMC2_DAT0);


        /* set for mmc2_dat2 and dat3 - both parts of the padconf register
          * Should set (x is don't change):  xxxx xxxx xxx1 1000 xxxx xxxx xxx1 1000 */

        val= omap_readl(CONTROL_PADCONF_MMC2_DAT2);
        val &=0xFFE0FFE0;
        val |=0x00180018 ;
        omap_writel( val,CONTROL_PADCONF_MMC2_DAT2);

	return 0;
}

/*-----------------------------------------------------------------------------

Routine Name:

        InitInterrupt

Routine Description:

        this function init the interrupt to the Wlan ISR routine.

Arguments:

        tnet_drv - Golbal Tnet driver pointer.


Return Value:

        status

-----------------------------------------------------------------------------*/

int hPlatform_initInterrupt(void *tnet_drv, void* handle_add ) 
{
	TWlanDrvIfObj *drv = tnet_drv;
    int rc;

	if (drv->irq == 0 || handle_add == NULL)
	{
	  print_err("hPlatform_initInterrupt() bad param drv->irq=%d handle_add=0x%x !!!\n",drv->irq,(int)handle_add);
	  return -EINVAL;
	}
	printk("hPlatform_initInterrupt: call omap_request_gpio %d\n", IRQ_GPIO);
	if (omap_request_gpio(IRQ_GPIO) != 0) 
    {
	    print_err("hPlatform_initInterrupt() omap_request_gpio() FAILED !!\n");
		return -EINVAL;
	}
	omap_set_gpio_direction(IRQ_GPIO, GPIO_DIR_INPUT);
#ifdef USE_IRQ_ACTIVE_HIGH
	printk("hPlatform_initInterrupt: call request_irq IRQF_TRIGGER_RISING\n");
	if ((rc = request_irq(drv->irq, handle_add, IRQF_TRIGGER_RISING, drv->netdev->name, drv)))
#else
	printk("hPlatform_initInterrupt: call request_irq IRQF_TRIGGER_FALLING\n");
	if ((rc = request_irq(drv->irq, handle_add, IRQF_TRIGGER_FALLING, drv->netdev->name, drv)))
#endif
	{
	    print_err("TIWLAN: Failed to register interrupt handler\n");
		omap_free_gpio(IRQ_GPIO);
		return rc;
	}

	if (omap_request_gpio(PMENA_GPIO) != 0) 
	{
		printk(KERN_ERR "%s:OMAP2430_TNETW_Power() omap_request_gpio FAILED\n",__FILE__);
	    omap_free_gpio(IRQ_GPIO);
		return -EINVAL;
	};
	omap_set_gpio_direction(PMENA_GPIO, GPIO_DIR_OUTPUT);

	return rc;

} /* hPlatform_initInterrupt() */

/*--------------------------------------------------------------------------------------*/

void hPlatform_freeInterrupt(void) 
{
	//omap_free_gpio(PMENA_GPIO);
	omap_free_gpio(IRQ_GPIO);
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


void hPlatform_Wlan_Hardware_DeInit(void)
{
  omap_free_gpio(PMENA_GPIO);

}


