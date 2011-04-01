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
/*																		   */
/*		MODULE:	rx.h													   */
/*    	PURPOSE:	Rx module functions header file						   */
/*																		   */
/***************************************************************************/
#ifndef _RX_DATA_H_
#define _RX_DATA_H_

#include "paramOut.h"
#include "TWDriver.h"
#include "wlanLinks.h"

#define DEF_EXCLUDE_UNENCYPTED				TI_FALSE
#define DEF_EAPOL_DESTINATION				OS_ABS_LAYER
#define DEF_RX_PORT_STATUS					CLOSE

typedef struct 
{
	TI_UINT32		excludedFrameCounter;	
	TI_UINT32		rxDroppedDueToVLANIncludedCnt;    
    TI_UINT32		rxWrongBssTypeCounter;
	TI_UINT32		rxWrongBssIdCounter;
    TI_UINT32      rcvUnicastFrameInOpenNotify;
}rxDataDbgCounters_t;

typedef struct 
{
    TI_UINT32      recvFromWlan;
	TI_UINT32      recvBytesFromWlan;
    TI_UINT32      discardWrongHlid;
    TI_UINT32      discardHlidClose;
    TI_UINT32      sendToHostapd;
    TI_UINT32      discardBeaconRoleAp;
    TI_UINT32      sendToMlme;
    TI_UINT32      sendToDataDispatcher;
    TI_UINT32      discardUnknownClass;
}rxDataLinkCounters_t;

/*                         |                           |                         |
 31 30 29 28 | 27 26 25 24 | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 | 11 10 9 8 | 7 6 5 4 | 3 2 1 0
                           |                           |                         |
*/                                           


typedef enum
{
	DATA_IAPP_PACKET  = 0,
	DATA_EAPOL_PACKET = 1,
	DATA_DATA_PACKET  = 2,
    DATA_VLAN_PACKET  = 3,
	MAX_NUM_OF_RX_DATA_TYPES
}rxDataPacketType_e;


/* Rx Link Info */
/* Rx connection states updated by the connection SM. */
typedef enum
{
	RX_CONN_STATE_CLOSE,
	RX_CONN_STATE_OPEN
} ERxConnState;

typedef struct 
{
    EWlanLinkType eType;        /* Link type */
    ERxConnState  eState;       /* Link state */
	EWlanLinkRole eRole;        /* Link Role type */
} TRxLinkInfo;

typedef void (*rxData_pBufferDispatchert) (TI_HANDLE hRxData , void *pBuffer, TRxAttr *pRxAttr);


typedef struct 
{
	/* Handles */
	TI_HANDLE	 		hCtrlData;
	TI_HANDLE	 		hTWD;
	TI_HANDLE			hMlme;
	TI_HANDLE			hOs;
	TI_HANDLE			hRsn;
	TI_HANDLE			hReport;
	TI_HANDLE			hSiteMgr;
	TI_HANDLE			hCcxMgr;
    TI_HANDLE           hEvHandler;
    TI_HANDLE           hTimer;
    TI_HANDLE           RxEventDistributor;
	TI_HANDLE           hThroughputTimer;
	TI_HANDLE			hPowerMgr;
#ifdef AP_MODE_ENABLED
    TI_HANDLE			hRoleAp;
#endif
    TI_BOOL             rxThroughputTimerEnable;
	TI_BOOL             rxDataExcludeUnencrypted;
    TI_BOOL             rxDataExludeBroadcastUnencrypted;
	eapolDestination_e 	rxDataEapolDestination;

	portStatus_e  		rxDataPortStatus;
	
    /* Rx Data Filters */
    filter_e            filteringDefaultAction;
    TI_BOOL             filteringEnabled;
    TI_BOOL             isFilterSet[MAX_DATA_FILTERS];
    TRxDataFilterRequest filterRequests[MAX_DATA_FILTERS];

	/* Counters */
	rxDataCounters_t	rxDataCounters;
	rxDataDbgCounters_t	rxDataDbgCounters;

	rxData_pBufferDispatchert rxData_dispatchBuffer[MAX_NUM_OF_RX_PORT_STATUS][MAX_NUM_OF_RX_DATA_TYPES];

	TI_INT32				prevSeqNum;

 	TI_UINT32           uLastDataPktRate;  /* save Rx packet rate for statistics */

	TI_BOOL			    reAuthInProgress;
	TI_HANDLE			reAuthActiveTimer;
	TI_UINT32			reAuthActiveTimeout;

	/* Info per link */
	TRxLinkInfo          aRxLinkInfo[WLANLINKS_MAX_LINKS];   /* information per link */
	rxDataLinkCounters_t rxDataLinkCounters[WLANLINKS_MAX_LINKS]; /* counters per link */


    /* Generic Ethertype support */
    TI_UINT16           genericEthertype;
    /* Bss Bridge Support*/
    TI_BOOL             bssBridgeEnable;
    TI_HANDLE           hTxDataQueue;
}rxData_t;

/* Public function definition */
/** 
 * \fn     rxData_IntraBssBridge_Enable 
 * \brief  Enable Intra BSS Bridge in Rx path
 * 
 * 
 * \return N/A
 */
extern void rxData_IntraBssBridge_Enable(TI_HANDLE hRxData);
/** 
 * \fn     rxData_IntraBssBridge_Disable 
 * \brief  Disable (Isolate) Intra BSS Bridge in Rx path
 * 
 * 
 * \return N/A
 */
extern void rxData_IntraBssBridge_Disable(TI_HANDLE hRxData);

#endif
