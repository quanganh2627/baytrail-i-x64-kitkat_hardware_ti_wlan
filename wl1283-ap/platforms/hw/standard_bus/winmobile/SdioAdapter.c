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
 
/** \file   SdioAdapter.c 
 *  \brief  The SDIO driver adapter. Platform dependent. 
 * 
 * An adaptation layer between WLAN core the TI Standard Sdio Bus Driver and lower SDIO driver (in BSP).
 *  
 *  \see    SdioAdapter.h, StdSdioBusDrv.c & h
 */
#include <windows.h>
#include <ndis.h>
#include <omap2430.h>


#include "TxnDefs.h"
#include "SdioAdapter.h"
#include "osApi.h"
#include "osDebug.h"

/************************************************************************
 * Debug Defines
 ************************************************************************/

#define TIWLAN_MMC_MAX_DMA                 8192
#define SSD_DEBUG

typedef enum {
	SSD_DEBUGLEVEL_EMERG=1,
	SSD_DEBUGLEVEL_ALERT,
	SSD_DEBUGLEVEL_CRIT,
	SSD_DEBUGLEVEL_ERR=4,
	SSD_DEBUGLEVEL_WARNING,
	SSD_DEBUGLEVEL_NOTICE,
	SSD_DEBUGLEVEL_INFO,
	SSD_DEBUGLEVEL_DEBUG=8
} ssd_debuglevel;


#ifdef SSD_DEBUG

#define PDEBUG(fmt)if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG)  os_printf(fmt)
#define PDEBUG1(fmt,p1) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG) os_printf(fmt,p1)
#define PDEBUG2(fmt,p1,p2) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG) os_printf(fmt,p1,p2)
#define PDEBUG3(fmt,p1,p2,p3) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG) os_printf(fmt,p1,p2,p3)
#define PDEBUG4(fmt,p1,p2,p3,p4) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG) os_printf(fmt,p1,p2,p3,p4)
#define PDEBUG5(fmt,p1,p2,p3,p4,p5) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_DEBUG) os_printf(fmt,p1,p2,p3,p4,p5)


#else

#define PDEBUG(fmt)
#define PDEBUG1(fmt,p1) 
#define PDEBUG2(fmt,p1,p2) 
#define PDEBUG3(fmt,p1,p2,p3) 
#define PDEBUG4(fmt,p1,p2,p3,p4) 
#define PDEBUG5(fmt,p1,p2,p3,p4,p5) 

#endif

#define PERR(fmt)if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR)  os_printf(fmt)
#define PERR1(fmt,p1) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR) os_printf(fmt,p1)
#define PERR2(fmt,p1,p2) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR) os_printf(fmt,p1,p2)
#define PERR3(fmt,p1,p2,p3) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR) os_printf(fmt,p1,p2,p3)
#define PERR4(fmt,p1,p2,p3,p4) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR) os_printf(fmt,p1,p2,p3,p4)
#define PERR5(fmt,p1,p2,p3,p4,p5) if(g_ssd_debug_level >= SSD_DEBUGLEVEL_ERR) os_printf(fmt,p1,p2,p3,p4,p5)

/************************************************************************
 * Defines
 ************************************************************************/

/* Bus Driver Registry name */
#define BUS_DRIVER_NAME  L"TIS1:"

/* Time out for loading Bus Driver */
#define TI_BUS_DRIVER_MODULE_WAITING_TIME_SECONDS (60*3)  /* 3 minutes for now */
#define WAIT_RELEASE (100)

/*IOCTL's*/
#define TI_STD_SDIO_BUS_DRV_GET_FUNCTIONS (100)
#define IOCTL_SHC_SET_BULTIN_CARD_STATE_INSERT  CTL_CODE(65432, 2345, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SHC_SET_BULTIN_CARD_STATE_REMOVE  CTL_CODE(65432, 2346, METHOD_BUFFERED, FILE_ANY_ACCESS)
/************************************************************************
 * Types
 ************************************************************************/

/*
 * An IO control "TI_STD_SDIO_BUS_DRV_GET_FUNCTIONS" which recieves a list of direct functions
 * of loaded API
 */
typedef struct {
    // See stdSdioBusDrv_ConnectBus
    int 	   (*connectBus)  (void *        fCbFunc,
                               void *        hCbArg,
                               unsigned int  uBlkSizeShift);
    // See stdSdioBusDrv_DisconnectBus
    int        (*disconnectBus)(void);
    // See stdSdioBusDrv_Transact
    ETxnStatus (*transact     )(unsigned int  uFuncId,
                                unsigned int  uHwAddr,
                                void *        pHostAddr,
                                unsigned int  uLength,
                                unsigned int  bDirection,
                                unsigned int  bBlkMode,
                                unsigned int  bFixedAddr,
                                unsigned int  bMore);
    // See stdSdioBusDrv_TransactBytes
    ETxnStatus (*transactBytes)(unsigned int  uFuncId,
                                unsigned int  uHwAddr,
                                void *        pHostAddr,
                                unsigned int  uLength,
                                unsigned int  bDirection,
                                unsigned int  bMore);
} stdSdioBusDrvFuncs_t;

/* Debug Level */
#ifdef SSD_DEBUG
	int g_ssd_debug_level = SSD_DEBUGLEVEL_DEBUG;
#else
	int g_ssd_debug_level = SSD_DEBUGLEVEL_ERR;
#endif

/* Locating the drivers functions */
static stdSdioBusDrvFuncs_t  g_stdSdioFuncs;
static stdSdioBusDrvFuncs_t *p_stdSdioFuncs = NULL;
static unsigned char *pDmaBufAddr = 0;
DMA_ADAPTER_OBJECT dma_adapter;
PHYSICAL_ADDRESS   dma_logical_add;

int sdioAdapt_ConnectBus (void *        fCbFunc,
                          void *        hCbArg,
                          unsigned int  uBlkSizeShift,
                          unsigned int  uSdioThreadPriority,
                          unsigned char **pRxDmaBufAddr,
                          unsigned int  *pRxDmaBufLen,
                          unsigned char **pTxDmaBufAddr,
                          unsigned int  *pTxDmaBufLen)

 {

     int rc = 0;
     
     /* Wait for card driver to be loaded */
    HANDLE busDriver = INVALID_HANDLE_VALUE;
    int initWaitMsec = TI_BUS_DRIVER_MODULE_WAITING_TIME_SECONDS*(1000/WAIT_RELEASE);
    DWORD numberOfBytesRead = 0;



    PDEBUG("Standard SDIO Adapter Bus Connection\n");


    dma_adapter.ObjectSize    = sizeof(dma_adapter);
    dma_adapter.InterfaceType = Internal;
    dma_adapter.BusNumber     = 0;
    if (pDmaBufAddr == 0)
    {
     pDmaBufAddr = (char *)HalAllocateCommonBuffer(&dma_adapter, TIWLAN_MMC_MAX_DMA, &dma_logical_add, FALSE);
     if (pDmaBufAddr == NULL)return 1;
    }

    *pRxDmaBufAddr = *pTxDmaBufAddr = pDmaBufAddr;

    *pRxDmaBufLen = *pTxDmaBufLen = TIWLAN_MMC_MAX_DMA;
   
    if (p_stdSdioFuncs != NULL)
    {
        PERR("SDIOAdapt: Warning! Function struct already filled!\n");
        p_stdSdioFuncs = NULL;
    }
       

    PDEBUG("sdioAdapt_ConnectBus: Bus Driver opening\n");
    do
    {
      /* busDriver = CreateFile(BUS_DRIVER_NAME, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); */
		  busDriver = CreateFile(BUS_DRIVER_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

		  PDEBUG1("sdioAdapt_ConnectBus: CreateFile returns %d\n", GetLastError());

          if (busDriver != INVALID_HANDLE_VALUE)
              break;

          initWaitMsec--;
          /*PDEBUG1("sdioAdapt_ConnectBus: Wait %d\n", initWaitMsec);*/
          Sleep(WAIT_RELEASE);
      } while (initWaitMsec > 0);
    

    if (busDriver == INVALID_HANDLE_VALUE)
    {
        PERR("sdioAdapt_ConnectBus: Couldn't open TI Bus Driver\n");
        /* Error */
        return 1;
    } else 
    {        
        p_stdSdioFuncs = &g_stdSdioFuncs;

        /* Get a functions list */
        if (0 == DeviceIoControl(busDriver, TI_STD_SDIO_BUS_DRV_GET_FUNCTIONS, 
                                 NULL, 0, p_stdSdioFuncs, sizeof(stdSdioBusDrvFuncs_t), 
                                 &numberOfBytesRead, NULL))
        {
            PERR("sdioAdapt_ConnectBus: Couldn't get function list\n");
            CloseHandle(busDriver);
			
            /* Erorr */
            return 1;
        }
        CloseHandle(busDriver);

        return p_stdSdioFuncs->connectBus(fCbFunc, hCbArg, uBlkSizeShift);
    }
}

int sdioAdapt_DisconnectBus (void)
{

    if (p_stdSdioFuncs == NULL)
    {
                  
      PERR("sdioAdapt_DisconnectBus() - Function pointer NULL !!!\n");
             
      /* Erorr */
      return 1;
    }

    if (pDmaBufAddr)
    {
        HalFreeCommonBuffer(&dma_adapter, TIWLAN_MMC_MAX_DMA, dma_logical_add,pDmaBufAddr, FALSE);
        pDmaBufAddr = 0;
    }

    return p_stdSdioFuncs->disconnectBus();
}

ETxnStatus sdioAdapt_Transact (unsigned int  uFuncId,
                               unsigned int  uHwAddr,
                               void *        pHostAddr,
                               unsigned int  uLength,
                               unsigned int  bDirection,
                               unsigned int  bBlkMode,
                               unsigned int  bFixedAddr,
                               unsigned int  bMore)
{

  
  if (p_stdSdioFuncs == NULL)
  {  
      PERR("sdioAdapt_Transact() - Function pointer NULL !!!\n");  
      return TXN_STATUS_ERROR;
  } 
  return p_stdSdioFuncs->transact(uFuncId, uHwAddr, pHostAddr, uLength, bDirection, bBlkMode, bFixedAddr, bMore);
}
         
ETxnStatus sdioAdapt_TransactBytes (unsigned int  uFuncId,
                                    unsigned int  uHwAddr,
                                    void *        pHostAddr,
                                    unsigned int  uLength,
                                    unsigned int  bDirection,
                                    unsigned int  bMore)
{
    if (p_stdSdioFuncs == NULL)
    {
      PERR("sdioAdapt_TransactBytes() - Function pointer NULL !!!\n");
      return TXN_STATUS_ERROR;
    }
    
    return p_stdSdioFuncs->transactBytes(uFuncId, uHwAddr, pHostAddr, uLength, bDirection, bMore);
}
