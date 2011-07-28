/*
 * host_platform.c
 *
 * Copyright(c) 1998 - 2010 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tidef.h"
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/wifi_tiwlan.h>
#include <linux/completion.h>
#include <linux/gpio.h>

#include "host_platform.h"
#include "ioctl_init.h"
#include "WlanDrvIf.h"
#include "Device1273.h"
#include "SdioDrv.h"


#define OS_API_MEM_ADDR  	0x0000000
#define OS_API_REG_ADDR  	0x0300000

#define SDIO_ATTEMPT_LONGER_DELAY_LINUX  150

static DECLARE_COMPLETION(sdio_ready);
static DECLARE_COMPLETION(wifi_ready);

static struct wifi_platform_data *wifi_control_data = NULL;
static struct resource *wifi_irqres = NULL;

/*
 * Driver callback (and context) to invoke when kernel
 * requests to suspend/resume the device
 */
static int (*wifi_suspend_cb)(TI_HANDLE) = NULL;
static int (*wifi_resume_cb)(TI_HANDLE) = NULL;
static TI_HANDLE wifi_susres_ctx;


static int wifi_probe(struct platform_device *pdev)
{
	int i, ret = 0;
	struct wifi_platform_data *wifi_ctrl = (struct wifi_platform_data *)(pdev->dev.platform_data);

	wifi_irqres = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

	if (wifi_irqres) {
		printk(KERN_INFO "TIWLAN: got wlan irq %lu\n",
			(unsigned long)(wifi_irqres->start));
		printk(KERN_INFO "TIWLAN: got wlan irq trigger flag '%s'\n",
			wifi_irqres->flags & IRQF_TRIGGER_FALLING ? "falling" : "unknown");
	} else {
		printk(KERN_ERR "TIWLAN: no resources!\n");
		return -ENODEV;
	}

	for (i = 0; i < wifi_ctrl->gpio_num; i++) {
		int gpio = wifi_ctrl->gpio[i] & 0xFFFF;
		int dir = (wifi_ctrl->gpio[i] >> 16) & 0x1;

		printk(KERN_INFO "TIWLAN: Request GPIO %d\n", gpio);
		ret = gpio_request(gpio, "ti_wlan");
		if (ret < 0)
			goto request_gpio_failed;

		if (dir == DIRECTION_OUT)
			gpio_direction_output(gpio, 0);
		else
			gpio_direction_input(gpio);
	}

	wifi_control_data = wifi_ctrl;

	complete(&wifi_ready);

	return 0;

request_gpio_failed:
	for (; i > 0; i--) {
		gpio_free(wifi_ctrl->gpio[i-1] & 0xFFFF);
	}
	return ret;
}

static int wifi_remove(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < wifi_control_data->gpio_num; i++) {
		int gpio = wifi_control_data->gpio[i] & 0xFFFF;

		printk(KERN_INFO "TIWLAN: Release GPIO %d\n", gpio);
		gpio_free(gpio);
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

static int wifi_add_dev(void)
{
	return platform_driver_register(&wifi_device);
}

static void wifi_del_dev(void)
{
	platform_driver_unregister(&wifi_device);
}

static int unplug_device(int delay)
{
	printk(KERN_INFO "%s\n", __func__);
	sdioDrv_ReleaseHost(SDIO_WLAN_FUNC);

	if(wifi_control_data) {
		if(wifi_control_data->set_reset)
			wifi_control_data->set_reset(1);
		if(wifi_control_data->set_power)
			wifi_control_data->set_power(0);
		/* sdioDrv_DetectChange(); */
	}

    mdelay(delay);

	return 0;
}
int hPlatform_DevicePowerOff (void)
{
	printk(KERN_INFO "%s\n", __func__);
	return unplug_device(10);
}

int hPlatform_DevicePowerOffSetLongerDelay(void)
{
	printk(KERN_INFO "%s\n", __func__);
	return unplug_device(SDIO_ATTEMPT_LONGER_DELAY_LINUX);
}

int hPlatform_DevicePowerOn (void)
{
	printk(KERN_INFO "%s\n", __func__);
	if(!wifi_control_data || !wifi_control_data->set_power)
		return -1;

	init_completion(&sdio_ready);
	sdioDrv_DetectChange();

	wifi_control_data->set_power(0);
	mdelay(500);
	wifi_control_data->set_power(1);
	/* New Power Up Sequence */
	mdelay(15);
	wifi_control_data->set_power(0);
	mdelay(1);
	wifi_control_data->set_power(1);
	/* Should not be changed, 50 msec cause failures */
	mdelay(100);

	if(wifi_control_data->set_reset)
		wifi_control_data->set_reset(0);

	/* let the mmc core finish enumeration + initialization before we continue */
	printk(KERN_INFO "%s: waiting for completion\n", __func__);
	wait_for_completion(&sdio_ready);
	sdioDrv_ClaimHost(SDIO_WLAN_FUNC);

	return 0;
}


static void hPlatform_sdio_ready(void)
{
	printk(KERN_INFO "%s\n", __func__);
	complete(&sdio_ready);
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
	printk(KERN_INFO "%s\n", __func__);
	
	printk(KERN_INFO "%s:Drv register Notif start\n", __func__);
	sdioDrv_Register_Notification(hPlatform_sdio_ready);
	printk(KERN_INFO "%s:Drv register Notif end\n", __func__);
	
	printk(KERN_INFO "%s:wifi_add_dev start\n", __func__);
	wifi_add_dev();
	printk(KERN_INFO "%s:wifi_add_dev end\n", __func__);
	
	/* make sure wifi device finished initialization before we move on */
	
	printk(KERN_INFO "%s: waiting for completion\n", __func__);
	wait_for_completion(&wifi_ready);

	if (!wifi_irqres)
		return -1;

	drv->irq = wifi_irqres->start;
	drv->irq_flags = wifi_irqres->flags & IRQF_TRIGGER_MASK;

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

int hPlatform_initInterrupt(void *tnet_drv, void* handle_add) 
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

	drv->irq_wake = 1;
	if (set_irq_wake(drv->irq, 1) != 0) {
		printk(KERN_INFO "TIWLAN: IRQ wake not implemented on platform\n");
		drv->irq_wake = 0;
	}

	return rc;
} /* hPlatform_initInterrupt() */

/*--------------------------------------------------------------------------------------*/

void hPlatform_freeInterrupt(void *tnet_drv)
{
	TWlanDrvIfObj *drv = tnet_drv;

	if (drv->irq_wake) {
		set_irq_wake(drv->irq, 0);
		drv->irq_wake = 0;
	}

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
void *hPlatform_hwGetMemoryAddr(TI_HANDLE OsContext)
{
	return (void *)OS_API_MEM_ADDR;
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
