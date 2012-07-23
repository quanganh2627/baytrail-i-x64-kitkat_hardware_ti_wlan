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


/** \file  TIWlanHpa.h
 *  \brief  Interface between the sandbox to the BSP of Omap2430
 *
 *  \see   
 */

#ifndef _TIWLANHPA_
#define _TIWLANHPA_

#include <shared_gpio.h> 
#include <assp.h>

#include "wlanhpa.h"


#define RST_GPIO_NUM	10 
#define RST_MOD_NUM		1

#define IRQ_GPIO_NUM	9 
#define IRQ_MOD_NUM		1


#define GPIO_PIN_MGR			OmapGpioPinMgr
#define GPIO_GPIO_MODULE	 	TOmapGpioModule
#define GPIO_MODULE_MGR 		OmapGpioModuleMgr
#define GPIO_PIN_HANDLE 		TOmapGpioPinHandle
#define	GPIO_INPUT_PIN_WRAPPER 	TOmapGpioInputPinWrapper	
#define GPIO_OUTPUT_PIN_WRAPPER TOmapGpioOutputPinWrapper

#define ASIC_CLASS					TOmap
#define ASIC_CLASS_PLAT				TOmapPlat	

#define PIN_TO_HANDLE OMAPGPIO_GPINID_TO_PHANDLE
#define INVALID_HANDLE 				KOmapGpioInvalidHandle

#if (INTERRUPT_DETECTION_TYPE==EDGE)
	#define IRQ_TYPE						EOmapGpioEDTEdge
	#warning "IRQ type is EDGE"

#elif (INTERRUPT_DETECTION_TYPE==LEVEL)
	#define IRQ_TYPE						EOmapGpioEDTLevel
	#warning "IRQ type is LEVEL"

#else
	#error "IRQ type not set"
#endif


class TIWlanHpa : public WlanHpa
{
public:

	/**
	 * Costructor.
	 */
	TIWlanHpa(MWlanOsaExt& aOsaExt);

	/**
	 * Destructor
	 */
	virtual ~TIWlanHpa();

	/**
	 * Turns WLAN devices power on
	 */
	virtual void PowerOnDevice();

	/**
	 * Turns WLAN devices power off
	 */
	virtual void PowerOffDevice();

	/**
	 * Called by hpa layer object client when peripheral interrupt
	 * informed by OnInterrupt method has been serviced.
	 * NOTE: on systems implementing level sensitive interrupt handling
	 * host interrupt is enabled at this state
	 */
	virtual void InterruptServiced();

	    /**
	 * Configures the HPA layer
	 */
	virtual void Configure( const WlanHpa::TConfig& aConfig );  

	/**
	 * Enables Irq
	 *
	 * NOTE: usage limited to bootup sequence only. 
	 * Untill this call the host side interrupts are disabled
	 *
	 */
	 virtual void EnableIrq();
	 
	 /**
	 * Toggles debug GPIO
	 * Usage limited to R&D (debug) purpose only
	 */
	 virtual TBool DebugGpioToggle( TBool aHigh );

private:

	/*
	* DFC - function called by a DFC to handle the interrupt
	* call another function to get in the correct class and
	* therefore don't need me-> all the time
	* Doesn't always work having me-> for the C software
	*/
	static void InterruptHandlerDfc(TAny *aPtr);

	/**
	* Start the DFC queue
	*/
	void InterruptHandlerStartDfc();

	
	/*
	* Call the actual interrupt handling routine and
	* re-enable the interrupts.
	*
	* The interrupt handler is in the third party software
	* block and performs the necessary functions
	*/
	void InterruptHandler();


	/**
	*  Disables interrupts and 
	*  unbinds the interrupt service routine.
	*/
	void KillInterrupts();

	/**
	* Sets GPIO as input line, binds it to an interrupt. Static function
	* Isr() is bound to the interrupt service routine. 
	*
	* Uses following defines:
	*
	*	#define IRQ_GPIO_NUM	9 
	*	#define IRQ_MOD_NUM		1
	* 
	* of which IRQ_GPIO_NUM is the GPIO number and 
	* IRQ_MOD_NUM is the module number where the corresponding
	* GPIO belongs to.
	*/		                   
	void SetupIrqHw( WlanHpa::TIsrPolarity aPolarity);

	/**
	* Sets GPIO as output line. Used for controlling the PMEN signal of the WLAN chip.
	#define RST_GPIO_NUM	10 
	#define RST_MOD_NUM		1

	* of which RST_GPIO_NUM is the GPIO number and 
	* RST_MOD_NUM is the module number where the corresponding
	* GPIO belongs to.

	*/		  

	void SetupRstHw			();


	/**
	* Interrupt handler function. 
	* Is bound to the ISR and handles continues serving the interrupt.
	*
	* \param aPtr HPA object context.
	* \param aPinHandle is not used. 
	*/	
	static void Isr			(TAny* aPtr, GPIO_PIN_HANDLE aPinHandle);

	/**
	* Clears WLAN PMEN line.
	*
	*/
	void EnterReset();

	/**
	* Sets WLAN PMEN line.
	*
	*/
	void ExitReset();

	/**
	* Disables WLAN interrupt notification.
	*
	*/
	void Disable();

	/**
	* Enables WLAN interrupt notification.
	*
	*/
	void Enable();

private:

	TDfc					   				 iIrqDfc;
	GPIO_INPUT_PIN_WRAPPER	iIrqPinWrapper;
	GPIO_OUTPUT_PIN_WRAPPER	iRstPinWrapper;
	WlanHpa::TConfig				iConfig;
};



#endif //_TIWLANHPA_

