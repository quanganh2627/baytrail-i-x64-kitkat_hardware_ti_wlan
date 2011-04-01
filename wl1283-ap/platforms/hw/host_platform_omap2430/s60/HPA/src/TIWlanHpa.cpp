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


/** \file  TIWlanHpa.cpp 
 *  \brief  Interface between the sandbox to the BSP of Omap2430
 *
 *  \see   
 */
#include "kernel.h"
#include "TIWlanHpa.h"
#include "wlanhpa.h"
#include "wlanosaext.h"

/* To debug HPA, define some prints here using OSA*/
#define     TRACE(msg) 
#define     ERROR_TRACE(msg) Kern::Printf(msg);

 
extern void Assert( const TInt8*, TInt, TInt );

/** 
 * \fn     Create
 * \brief  static creator
 * 
 * Create hpa layer object
 *
 * \note   
 * \param aOsaExt osa extension object
 * \return hpa layer object, NULL upon failure
 */ 
WlanHpa* WlanHpa::Create( MWlanOsaExt& aOsaExt )
{
    TRACE(("WlanHpa::Create\n"));

    return new TIWlanHpa(aOsaExt);
}

/** 
 * \fn     Destroy
 * \brief  
 * 
 * Destroy hpa layer object
 *
 * \note   
 * \param aWlanHpa hpa layer object
 */ 
void WlanHpa::Destroy( WlanHpa* aWlanHpa )
{    
    if (aWlanHpa)
    {
        delete aWlanHpa;
    }
}

/** 
 * \fn     TIWlanHpa
 * \brief  constructor
 * 
 * \note   
 * \param  aOsaExt 
 */ 
TIWlanHpa::TIWlanHpa(MWlanOsaExt& aOsaExt):
	WlanHpa( aOsaExt ),
	iIrqDfc(InterruptHandlerDfc,(TAny*)this,0),
	iIrqPinWrapper( GPIO_INPUT_PIN_WRAPPER (0)),
	iRstPinWrapper( GPIO_OUTPUT_PIN_WRAPPER(0) )
{
        TRACE(("%s\n",__FUNCTION__));

    iIrqDfc.SetDfcQ( Kern::DfcQue0() );
}

/** 
 * \fn     TIWlanHpa
 * \brief  destructor
 * 
 * \note   
 */ 
TIWlanHpa::~TIWlanHpa()
{
        TRACE(("%s\n",__FUNCTION__));

}
 
/** 
 * \fn     TIWlanHpa
 * \brief  Turns WLAN device power on
 * 
 * \note   
 */ 
void TIWlanHpa::PowerOnDevice()
{
    TRACE(("PowerOnDevice\n")); 

    /* Hook to IRQ line */
    SetupIrqHw(iConfig.iIsrPolarity);
    /* Hook to Reset (PM Enable) line */
    SetupRstHw();

    /* Activate PM Enable line */
    ExitReset();
}

/** 
 * \fn     TIWlanHpa
 * \brief  Turns WLAN devices power off
 * 
 * \note   
 */ 
void TIWlanHpa::PowerOffDevice()
{
    TRACE(("%s\n",__FUNCTION__));

    /* Unbind ISR and disable notification */
    KillInterrupts();
    /* Deactivate PM Enable */
    EnterReset();

    /* Release reset & IRQ pins */
    GPIO_PIN_MGR::ReleasePin(PIN_TO_HANDLE(RST_GPIO_NUM));
    GPIO_PIN_MGR::ReleasePin(PIN_TO_HANDLE(IRQ_GPIO_NUM));

    /* cancel any pending interrupts (i.e DFC was scheduled before calling PowerOffDevice() ) */
    iIrqDfc.Cancel();
}

/** 
 * \fn     InterruptServiced
 * \brief  
 * 
 * Called by hpa layer object client when peripheral interrupt
 * informed by OnInterrupt method has been serviced.
 * NOTE: on systems implementing level sensitive interrupt handling
 * host interrupt is enabled at this state
 *
 * \note   
 */     
void TIWlanHpa::InterruptServiced()
{
    Enable();
}

/** 
 * \fn     Configure
 * \brief  configure polarity of IRQ
 * 
 * \note   
 * \param aWlanHpa::TConfig
 */ 
void TIWlanHpa::Configure( const WlanHpa::TConfig& aConfig )
{    
    /* Save configuration of polarity, to be used in SetupIrqHw() */
    iConfig = aConfig;
}

/** 
 * \fn     Configure
 * \brief  configure polarity of IRQ
 * 
 * \note   
 * \param aWlanHpa::TConfig
 */ 
void TIWlanHpa::EnableIrq()
{
    Enable();
}

/** 
 * \fn     DebugGpioToggle
 * \brief 
 * 
 * \note   
 * \param 
 */ 
TBool TIWlanHpa::DebugGpioToggle( TBool aHigh ){ return 0;}
/** 
 * \fn     ExitReset
 * \brief  Sets WLAN PMEN line.
 * 
 * \note   
 * \param  
 */ 
void TIWlanHpa::ExitReset()
{
    iRstPinWrapper.SetData();
}

/** 
 * \fn     EnterReset
 * \brief  Clears WLAN PMEN line.
 * 
 * \note   
 * \param  
 */ 
void TIWlanHpa::EnterReset()
{
    iRstPinWrapper.ClearData();
}

/** 
 * \fn     Disable
 * \brief  Disables WLAN interrupt notification.
 * 
 * \note   
 * \param  aOsaExt 
 */ 
void TIWlanHpa::Disable()
{
    iIrqPinWrapper.EnableInterruptNotif(EFalse);
}

/** 
 * \fn     Enable
 * \brief  Enables WLAN interrupt notification.
 * 
 * \note   
 * \param  
 */ 
void TIWlanHpa::Enable()
{
    iIrqPinWrapper.EnableInterruptNotif(ETrue);
}

/** 
 * \fn     SetupIrqHw
 * \brief  set IRQ line
 * 
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
 *
 * \note   
 * \param  
 */ 
void TIWlanHpa::SetupIrqHw( WlanHpa::TIsrPolarity aPolarity)
{
   /*--------------------------------------------------------*/
   /* common code to init the irq pin                        */
   /*--------------------------------------------------------*/
    GPIO_MODULE_MGR::Init();
    GPIO_GPIO_MODULE& module = GPIO_MODULE_MGR::Module( IRQ_MOD_NUM );
    module.Disable( EFalse );
    GPIO_PIN_HANDLE irqPinHandle = GPIO_PIN_MGR::AcquirePin( IRQ_GPIO_NUM );
    if (irqPinHandle == INVALID_HANDLE)
    {
        ERROR_TRACE(("ERROR:cant grab wlan interrupt pin"))
        Assert( __FILE__, __LINE__, EFalse );
    }
    GPIO_PIN_MGR::SetPinAsInput( irqPinHandle );
    iIrqPinWrapper = GPIO_INPUT_PIN_WRAPPER( irqPinHandle );
    iIrqPinWrapper.SetEventDetection( IRQ_TYPE, 
        (aPolarity == WlanHpa::EIsrPolarityHigh) ? EOmapGpioEDSUp : EOmapGpioEDSDown);

    TInt r = iIrqPinWrapper.BindIsr( Isr, this );    
    if (r != KErrNone)
    {
        ERROR_TRACE(("ERROR:cant bind wlan interrupt to pin"))
        Assert( __FILE__, __LINE__, EFalse );
    }
    iIrqPinWrapper.EnableWakeUpNotif(ETrue,ETrue);
    Disable(); // disable int until required

}
	
/** 
 * \fn     SetupRstHw
 * \brief  set reset line.
 * 
 * Sets GPIO as output line. Used for controlling the PMEN signal of the WLAN chip.
 * #define RST_GPIO_NUM	10 
 * #define RST_MOD_NUM		1
 *
 * of which RST_GPIO_NUM is the GPIO number and 
 * RST_MOD_NUM is the module number where the corresponding
 * GPIO belongs to.
 * \note   
 * \param  
 */ 
void TIWlanHpa::SetupRstHw()
{
    /*--------------------------------------------------------*/
    /* common code to init the rst pin                        */
    /*--------------------------------------------------------*/

    GPIO_MODULE_MGR::Init();
    GPIO_GPIO_MODULE& module = GPIO_MODULE_MGR::Module( RST_MOD_NUM );
    module.Disable( EFalse );

    GPIO_PIN_HANDLE RstPinHandle = GPIO_PIN_MGR::AcquirePin( RST_GPIO_NUM );
    if (RstPinHandle == INVALID_HANDLE)
    {
        ERROR_TRACE(("ERROR:cant grab wlan enable pin"))
        Assert( __FILE__, __LINE__, EFalse );
    }
    GPIO_PIN_MGR::SetPinAsOutput( RstPinHandle );
    iRstPinWrapper = GPIO_OUTPUT_PIN_WRAPPER( RstPinHandle );
    EnterReset();
}

/** 
 * \fn     Isr
 * \brief  Interrupt handler function. 
 *
 * \note   
 * \param aPtr HPA object context.
 * \param aPinHandle is not used. 
 */ 
void TIWlanHpa::Isr( TAny* aPtr, GPIO_PIN_HANDLE /*aPinHandle*/)
{
    TIWlanHpa* p = static_cast<TIWlanHpa*>( aPtr );

    p->iIrqPinWrapper.AckInterruptNotif();
    p->Disable();

    p->InterruptHandlerStartDfc();
}

/** 
 * \fn     InterruptHandlerStartDfc
 * \brief  Start the DFC queue
 * 
 * \note   
 * \param  
 */ 
 /* TODO yuval - why not from Isr ??? */
void TIWlanHpa::InterruptHandlerStartDfc()
{
    iIrqDfc.Add();
}

/** 
 * \fn     InterruptHandlerDfc
 * 
 * DFC - function called by a DFC to handle the interrupt
 * call another function to get in the correct class and
 * therefore don't need me-> all the time
 * Doesn't always work having me-> for the C software
 * \note   
 * \param  
 */ 
void TIWlanHpa::InterruptHandlerDfc(TAny* aPtr)
{
    TIWlanHpa* me = (TIWlanHpa*)aPtr;
    me->InterruptHandler();
}

 /** 
 * \fn     InterruptHandler
 * \brief  Call the actual interrupt handling routine and
 *
 * \note   
 * \param  
 */ 
void TIWlanHpa::InterruptHandler()
{
    OsaExtCb().MutexAcquire();
    HpaCb().OnInterrupt();
    OsaExtCb().MutexRelease();
}

 /** 
 * \fn     KillInterrupts
 * \brief  Call the actual interrupt handling routine and
 *
 *  Disables interrupts and 
 *  unbinds the interrupt service routine.
 *
 * \note   
 * \param  
 */ 
void TIWlanHpa::KillInterrupts()
{
    iIrqPinWrapper.EnableWakeUpNotif(EFalse,EFalse);
    iIrqPinWrapper.EnableInterruptNotif( EFalse );
    iIrqPinWrapper.UnbindIsr();
}

