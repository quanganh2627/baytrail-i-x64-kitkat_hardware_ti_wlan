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

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*      PURPOSE:    DataCtrl module api functions header file              */
/*                                                                         */
/***************************************************************************/

#ifndef _DATA_CTRL_API_H_
#define _DATA_CTRL_API_H_

#include "paramOut.h"
#include "rxXfer_api.h"
#include "802_11Defs.h"
#include "GeneralUtilApi.h"
#include "DrvMainModules.h"

/* Include all core Tx modules APIs */
#include "txCtrl_Api.h"
#include "txPort_Api.h"
#include "txDataQueue_Api.h"
#include "txMgmtQueue_Api.h"
#include "wlanLinks.h"
#include "rx.h"


typedef enum
{
    RX_DATA_EAPOL_DESTINATION_PARAM = 0x01,
    RX_DATA_PORT_STATUS_PARAM       = 0x02

} ERxDataParam;


/*  RX MODULE   */
/*--------------*/

/* Rx module interface functions */

#define RECV_OK                  0x1
#define DIRECTED_BYTES_RECV      0x2
#define DIRECTED_FRAMES_RECV     0x4
#define MULTICAST_BYTES_RECV     0x8
#define MULTICAST_FRAMES_RECV    0x10   
#define BROADCAST_BYTES_RECV     0x20    
#define BROADCAST_FRAMES_RECV    0x40

#define NO_RX_NOTIFICATION  0x0

#define ALL_RCV_FRAME (DIRECTED_FRAMES_RECV|MULTICAST_FRAMES_RECV|BROADCAST_FRAMES_RECV)

#define  MAX_RX_NOTIF_REQ_ELMENTS 8


/*TI_HANDLE rxData_create (msduReceiveCB_t* msduReceiveCB, TI_HANDLE hOs);  */
TI_HANDLE rxData_create (TI_HANDLE hOs);    

void      rxData_init (TStadHandlesList *pStadHandles);

TI_STATUS rxData_SetDefaults (TI_HANDLE hRxData, rxDataInitParams_t * rxDataInitParams);
                         
void      rxData_receivePacketFromWlan (TI_HANDLE hRxData, void *pBuffer, TRxAttr* pRxAttr);

TI_STATUS rxData_stop(TI_HANDLE hRxData);

TI_STATUS rxData_unLoad(TI_HANDLE hRxData); 

TI_STATUS ctrlData_getParamProtType(TI_HANDLE hCtrlData, erpProtectionType_e *protType);

TI_STATUS ctrlData_getParamPreamble(TI_HANDLE hCtrlData, EPreamble *preamble);

TI_STATUS ctrlData_getParamBssid(TI_HANDLE hCtrlData, EInternalParam paramVal, TMacAddr bssid);

TI_STATUS rxData_getParam(TI_HANDLE hRxData, paramInfo_t *pParamInfo);  

TI_STATUS rxData_setParam(TI_HANDLE hRxData, paramInfo_t *pParamInfo);  

TI_STATUS rxData_getTiwlnCounters(TI_HANDLE hRxData, TIWLN_COUNTERS *pTiwlnCounters);

void      rxData_resetCounters(TI_HANDLE hRxData);

TI_HANDLE rxData_RegNotif(TI_HANDLE hRxData,TI_UINT16 EventMask,GeneralEventCall_t CallBack,TI_HANDLE context,TI_UINT32 Cookie);

TI_STATUS rxData_UnRegNotif(TI_HANDLE hRxData,TI_HANDLE RegEventHandle);

TI_STATUS rxData_AddToNotifMask(TI_HANDLE hRxData,TI_HANDLE Notifh,TI_UINT16 EventMask);

void rxData_SetReAuthInProgress(TI_HANDLE hRxData, TI_BOOL	value);

TI_BOOL rxData_IsReAuthInProgress(TI_HANDLE hRxData);

void rxData_StopReAuthActiveTimer(TI_HANDLE hRxData);		
void rxData_ReauthDisablePriority(TI_HANDLE hRxData);
TI_STATUS rxData_mgmtPacketComplete (TI_HANDLE hRxData, void *pBuffer, TI_UINT32 uBufferLen, TI_BOOL bTxSuccess);

/* debug functions */
void rxData_resetDbgCounters(TI_HANDLE hRxData);
void rxData_resetLinkCounters(TI_HANDLE hRxData, TI_UINT32 uHlid);
void rxData_printRxBlock(TI_HANDLE hRxData);
void rxData_printRxCounters(TI_HANDLE hRxData);
void rxData_startRxThroughputTimer(TI_HANDLE hRxData); 
void rxData_stopRxThroughputTimer(TI_HANDLE hRxData); 
void rxData_printRxDataFilter(TI_HANDLE hRxData);
void rxData_SetLinkType (TI_HANDLE hRxData, TI_UINT32 uHlid, EWlanLinkType eLinkType, EWlanLinkRole eLinkRole);
void rxData_SetLinkState (TI_HANDLE hRxData, TI_UINT32 uHlid, ERxConnState eRxConnState);



/* CONTROL MODULE */
/*----------------*/

#define XFER_OK                 0x1
#define DIRECTED_BYTES_XFER     0x2
#define DIRECTED_FRAMES_XFER    0x4
#define MULTICAST_BYTES_XFER    0x8
#define MULTICAST_FRAMES_XFER   0x10
#define BROADCAST_BYTES_XFER    0x20
#define BROADCAST_FRAMES_XFER   0x40

#define  MAX_TX_NOTIF_REQ_ELMENTS 8


typedef struct
{
    TMacAddr    ctrlDataDeviceMacAddress; 
} ctrlDataConfig_t; 

/* retries for the next link test packet Callback */
typedef void (*retriesCB_t)(TI_HANDLE handle, TI_UINT8 ackFailures);

/*******************************/
/* Control module interface functions */
TI_HANDLE ctrlData_create(TI_HANDLE hOs);

void      ctrlData_init (TStadHandlesList *pStadHandles,                       
                         retriesCB_t       retriesUpdateCBFunc,
                         TI_HANDLE         retriesUpdateCBObj);

TI_STATUS ctrlData_SetDefaults (TI_HANDLE hCtrlData, ctrlDataInitParams_t *ctrlDataInitParams);

TI_STATUS ctrlData_unLoad(TI_HANDLE hCtrlData); 

TI_STATUS ctrlData_getParam(TI_HANDLE hCtrlData, paramInfo_t *pParamInfo);  

TI_STATUS ctrlData_setParam(TI_HANDLE hCtrlData, paramInfo_t *pParamInfo);  

TI_STATUS ctrlData_stop(TI_HANDLE hCtrlData);

TI_STATUS ctrlData_getTiwlnCounters(TI_HANDLE hCtrlData, TIWLN_COUNTERS *pTiwlnCounters);   

void ctrlData_updateTxRateAttributes(TI_HANDLE hCtrlData);	

void ctrlData_getCurrBssTypeAndCurrBssId(TI_HANDLE hCtrlData, TMacAddr *pCurrBssid, 
                                         ScanBssType_e *pCurrBssType);  

void ctrlData_txCompleteStatus(TI_HANDLE hCtrlData, TxResultDescriptor_t *pTxResultInfo,
							   EHwRateBitFiled HwTxRequestRate, TI_UINT8 txPktFlags);

void ctrlData_ToggleTrafficIntensityNotification (TI_HANDLE hCtrlData, TI_BOOL enabledFlag);

/** 
 * \fn     ctrlData_setDataTxRatePolicies
 * \brief  Configure Data frames (per AC) TX Rate policies to FW
 * 
 * \param   hCtrlData - handle to CtrlData object
 * \param uSupRatesBitmap - supported rates bitmap
 * 
 * \return  NA
 */ 
void ctrlData_setDataTxRatePolicies(TI_HANDLE hCtrlData, TI_UINT32 uSupRatesBitmap);

/** 
 * \fn     ctrlData_setBrcstTxRatePolicy
 * \brief  Configure Broadcast frames TX Rate policy to FW
 * 
 * \param   hCtrlData - handle to CtrlData object
 * \param uEnabledRates - supported rates bitmap
 * 
 * \return  NA
 */ 
void ctrlData_setBrcstTxRatePolicy(TI_HANDLE hCtrlData, TI_UINT32 uBrcstSupRatesBitmap);

/** 
 * \fn     ctrlData_setMgmtTxRatePolicy
 * \brief  Configure Management frames TX Rate policy to FW
 * 
 * \param   hCtrlData - handle to CtrlData object
 * \param uEnabledRates - supported rates bitmap
 * 
 * \return  NA
 */ 
void ctrlData_setMgmtTxRatePolicy(TI_HANDLE hCtrlData, TI_UINT32 uSupRatesBitmap);
void ctrlData_InitRatePolicy(TI_HANDLE hCtrlData);


/* dbg functions */
/*---------------*/
#ifdef TI_DBG
void ctrlData_printTxParameters(TI_HANDLE hCtrlData);
void ctrlData_printCtrlBlock(TI_HANDLE hCtrlData);
#endif /* TI_DBG */


#endif  /* _DATA_CTRL_API_H_ */

