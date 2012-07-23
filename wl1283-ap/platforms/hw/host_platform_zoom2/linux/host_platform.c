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
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/wifi_tiwlan.h>

#include "host_platform.h"
#include "ioctl_init.h"
#include "WlanDrvIf.h"
#include "Device1273.h"


#define OS_API_MEM_ADDR  	0x0000000
#define OS_API_REG_ADDR  	0x0300000
#if 0 /* needed for first time new host ramp*/ 
static void dump_omap_registers(void);
#endif

#define SDIO_ATTEMPT_LONGER_DELAY_LINUX  150

static struct wifi_platform_data *wifi_control_data = NULL;
static struct resource *wifi_irqres = NULL;

/*
 * Driver callback (and context) to invoke when kernel
 * requests to suspend/resume the device
 */
static int (*wifi_suspend_cb)(TI_HANDLE) = NULL;
static int (*wifi_resume_cb)(TI_HANDLE) = NULL;
static TI_HANDLE wifi_susres_ctx;


static int wifi_probe( struct platform_device *pdev )
{
	struct wifi_platform_data *wifi_ctrl = (struct wifi_platform_data *)(pdev->dev.platform_data);

	wifi_irqres = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "device_wifi_irq");

	if( wifi_ctrl ) {
		wifi_control_data = wifi_ctrl;
	}
	return 0;
}

static int wifi_remove( struct platform_device *pdev )
{
	struct wifi_platform_data *wifi_ctrl = (struct wifi_platform_data *)(pdev->dev.platform_data);

	if( wifi_ctrl ) {
		if( wifi_ctrl->set_carddetect )
			wifi_ctrl->set_carddetect(0);	/* CardDetect (1->0) */
		if( wifi_ctrl->set_reset )
			wifi_ctrl->set_reset(1);	/* Reset active */
		if( wifi_ctrl->set_power )
			wifi_ctrl->set_power(0);	/* Power Off */
	}
	return 0;
}

static int wifi_suspend(struct platform_device *dev, pm_message_t state)
{
	if (!wifi_suspend_cb)
	{
		printk("%s: cannot suspend - no callback registered\n", __func__);
		return -1;
	}

	return wifi_suspend_cb(wifi_susres_ctx);
}

static int wifi_resume(struct platform_device *dev)
{
	if (!wifi_resume_cb)
	{
		printk("%s: cannot resume - no callback registered\n", __func__);
		return -1;
	}

	return wifi_resume_cb(wifi_susres_ctx);
}

static struct platform_driver wifi_device = {
	.probe          = wifi_probe,
	.remove         = wifi_remove,
	.suspend        = wifi_suspend,
	.resume         = wifi_resume,
	.driver         = {
		.name   = "device_wifi",
	},
};

static int wifi_add_dev( void )
{
	return platform_driver_register( &wifi_device );
}

static void wifi_del_dev( void )
{
	platform_driver_unregister( &wifi_device );
}

int wifi_set_carddetect( int on )
{
	if( wifi_control_data && wifi_control_data->set_carddetect ) {
		wifi_control_data->set_carddetect(on);
	}
	return 0;
}

int wifi_set_power( int on, unsigned long msec )
{
	/* printk("%s = %d\n", __FUNCTION__, on); */
	if( wifi_control_data && wifi_control_data->set_power ) {
		wifi_control_data->set_power(on);
	}
	else {
		gpio_set_value(PMENA_GPIO_EXT, on);
		gpio_set_value(PMENA_GPIO, on);
	}
	if( msec )
		mdelay(msec);
	return 0;
}
int wifi_set_reset( int on, unsigned long msec )
{
	/* printk("%s = %d\n", __FUNCTION__, on); */
	if( wifi_control_data && wifi_control_data->set_reset ) {
		wifi_control_data->set_reset(on);
	}
	if( msec )
		mdelay(msec);
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
    if ((err = wifi_set_power(0, 500)) == 0) {
        /* Turn power ON*/
        err = wifi_set_power(1, 50);
    }

    return err;

} /* hPlatform_hardResetTnetw() */

/* Turn device power off */
int hPlatform_DevicePowerOff (void)
{
    int err;

    err = wifi_set_power(0, 10);

#ifndef PROPRIETARY_SDIO
    if(wifi_control_data->set_carddetect)
        wifi_control_data->set_carddetect(0);
#endif

    return err;
}


/* Turn device power off according to a given delay */
int hPlatform_DevicePowerOffSetLongerDelay(void)
{
    int err;

    err = wifi_set_power(0, SDIO_ATTEMPT_LONGER_DELAY_LINUX);

    return err;
}


/* Turn device power on */
int hPlatform_DevicePowerOn (void)
{
    int err;

    /* New Power Up Sequence */
    wifi_set_power(1, 15);
    wifi_set_power(0, 1);

    /* Should not be changed, 50 msec cause failures */
    err = wifi_set_power(1, 70);

#ifndef PROPRIETARY_SDIO
    if(wifi_control_data->set_reset)
        wifi_control_data->set_reset(0);
    if(wifi_control_data->set_carddetect)
        wifi_control_data->set_carddetect(1);

    /* let mmc core finish enumeration + initialization */
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);

#endif

    return err;
}

/*--------------------------------------------------------------------------------------*/

/*
 * \fn		hPlatform_SetupPm
 *
 * \param	suspend_cb	callback to invoke upon suspend
 * \param	resume_cb	callback to invoke upon resume
 * \param	susres_ctx	context for the suspend/resume callbacks
 */
void hPlatform_SetupPm(int (*suspend_cb)(void*), int (*resume_cb)(void*) , void* susres_ctx)
{
	/* setup the suspend/resume callbacks */
	wifi_suspend_cb = suspend_cb;
	wifi_resume_cb  = resume_cb;
	wifi_susres_ctx = susres_ctx;
}

int hPlatform_Wlan_Hardware_Init(void *tnet_drv)
{
    TWlanDrvIfObj *drv = tnet_drv;

    wifi_add_dev();
    if (wifi_irqres) {
        drv->irq = wifi_irqres->start;
        drv->irq_flags = wifi_irqres->flags & IRQF_TRIGGER_MASK;
    }
    else {
        drv->irq = TNETW_IRQ;
        drv->irq_flags = (unsigned long)IRQF_TRIGGER_FALLING;
    }

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

	if ((rc = request_irq(drv->irq, handle_add, drv->irq_flags, drv->netdev->name, drv)))
	{
		print_err("TIWLAN: Failed to register interrupt handler\n");
		return rc;
	}

	set_irq_wake(drv->irq, 1);

	return rc;
} /* hPlatform_initInterrupt() */

/*--------------------------------------------------------------------------------------*/

void hPlatform_freeInterrupt(void *tnet_drv) 
{
	TWlanDrvIfObj *drv = tnet_drv;

	set_irq_wake(drv->irq, 0);
	free_irq(drv->irq, drv);
}

/****************************************************************************************
 *                        hPlatform_hwGetMemoryAddr()                                 
 ****************************************************************************************
DESCRIPTION:	

ARGUMENTS:		

RETURN:			

NOTES:         	
*****************************************************************************************/
void* hPlatform_hwGetMemoryAddr(TI_HANDLE OsContext)
{
	return (void*)OS_API_MEM_ADDR;
}


void hPlatform_Wlan_Hardware_DeInit(void)
{
	wifi_del_dev();
}

#if 0/* needed for first time new host ramp*/
static void dump_omap_registers(void)
{
    printk(KERN_ERR "MMC3 CMD  addr 0x%x value is =%x\n", CONTROL_PADCONF_MMC3_CMD,    omap_readl( CONTROL_PADCONF_MMC3_CMD ));
    printk(KERN_ERR "MMC3 CLK  addr 0x%x value is =%x\n", CONTROL_PADCONF_MMC3_CLK,    omap_readl( CONTROL_PADCONF_MMC3_CLK ));
    printk(KERN_ERR "MMC3 DAT0 addr 0x%x value is =%x\n", CONTROL_PADCONF_MMC3_DAT0,   omap_readl( CONTROL_PADCONF_MMC3_DAT0 ));
    printk(KERN_ERR "MMC3 DAT2 addr 0x%x value is =%x\n", CONTROL_PADCONF_MMC3_DAT2,   omap_readl( CONTROL_PADCONF_MMC3_DAT2 ));
    printk(KERN_ERR "MMC3 DAT3 addr 0x%x value is =%x\n", CONTROL_PADCONF_MMC3_DAT3,   omap_readl( CONTROL_PADCONF_MMC3_DAT3 ));
    printk(KERN_ERR "WLAN_EN   addr 0x%x value is =%x\n", CONTROL_PADCONF_CAM_D1,      omap_readl( CONTROL_PADCONF_CAM_D1 ));
    printk(KERN_ERR "WLAN_IRQ  addr 0x%x value is =%x\n", CONTROL_PADCONF_MCBSP1_CLKX, omap_readl( CONTROL_PADCONF_MCBSP1_CLKX ));
    return;
}
#endif
