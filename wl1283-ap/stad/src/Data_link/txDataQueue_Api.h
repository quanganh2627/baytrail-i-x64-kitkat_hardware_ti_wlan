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
/*      PURPOSE:    Tx Data Queue module api functions header file         */
/*                                                                         */
/***************************************************************************/

#ifndef _TX_DATA_QUEUE_API_H_
#define _TX_DATA_QUEUE_API_H_

#include "paramOut.h"
#include "TWDriver.h"
#include "DrvMainModules.h"
#include "txMgmtQueue_Api.h"


/* 
 * Tx-Data-Queue functions:
 */
TI_HANDLE txDataQ_Create (TI_HANDLE hOs);
void      txDataQ_Init (TStadHandlesList *pStadHandles);
TI_STATUS txDataQ_SetDefaults (TI_HANDLE  hTxDataQ, txDataInitParams_t *pTxDataInitParams);
TI_STATUS txDataQ_Destroy (TI_HANDLE hTxDataQ);
void      txDataQ_ClearQueues (TI_HANDLE hTxDataQ);
TI_STATUS txDataQ_InsertPacket (TI_HANDLE hTxDataQ, TTxCtrlBlk *pPktCtrlBlk, TI_UINT8 uPacketDtag, TIntraBssBridge *pIntraBssBridgeParam);
void      txDataQ_StopQueue (TI_HANDLE hTxDataQ, TI_UINT32 tidBitMap);
void      txDataQ_StopLink (TI_HANDLE hTxDataQ, TI_UINT32 uHlid);
void      txDataQ_UpdateBusyMap (TI_HANDLE hTxDataQ, TI_UINT32 tidBitMap, TI_UINT32 uLinkBitMap);
void      txDataQ_StopAll (TI_HANDLE hTxDataQ);
void      txDataQ_WakeAll (TI_HANDLE hTxDataQ);
void      txDataQ_DisableLink (TI_HANDLE hTxDataQ, TI_UINT32 uHlid);
void      txDataQ_EnableLink (TI_HANDLE hTxDataQ, TI_UINT32 uHlid);

/*
 * Tx data queue per link functions:
 */
void      txDataQ_SetLinkType   (TI_HANDLE hTxDataQ, TI_UINT32 uHlid, EWlanLinkType eLinkType);
TI_STATUS txDataQ_LinkMacAdd (TI_HANDLE hTxDataQ, TI_UINT32 uHlid, TMacAddr tMacAddr);
void      txDataQ_LinkMacRemove (TI_HANDLE hTxDataQ, TI_UINT32 uHlid);
TI_STATUS txDataQ_LinkMacFind   (TI_HANDLE hTxDataQ, TI_UINT32 *uHlid, TMacAddr tMacAddr);
void TxDataQ_SetEncryptFlag(TI_HANDLE hTxDataQ, TI_UINT32  uHlid,int flag);
void TxDataQ_setEncryptionFieldSizes(TI_HANDLE hTxDataQ, TI_UINT32  uHlid,TI_UINT8 encryptionFieldSize);
TI_UINT8 TxDataQ_getEncryptionFieldSizes(TI_HANDLE hTxDataQ, TI_UINT32  uHlid);

void      txDataQ_GetBcastLink  (TI_HANDLE hTxDataQ, TI_UINT32 *uHlid);
/*
 * Tx data resources functions:
 */
TI_STATUS txDataQ_AllocCheckResources (TI_HANDLE hTxDataQ, TTxCtrlBlk *pPktCtrlBlk);
void      txDataQ_FreeResources       (TI_HANDLE hTxDataQ, TTxCtrlBlk *pPktCtrlBlk);
void      txDataQ_FlushLinkQueues     (TI_HANDLE hTxDataQ, TI_UINT32 uHlid);

#ifdef TI_DBG
void      txDataQ_PrintModuleParams    (TI_HANDLE hTxDataQ);
void      txDataQ_PrintQueueStatistics (TI_HANDLE hTxDataQ);
void      txDataQ_ResetQueueStatistics (TI_HANDLE hTxDataQ);
#endif /* TI_DBG */


/* 
 * Tx-Data-Classifier functions:
 */
TI_STATUS txDataClsfr_Config           (TI_HANDLE hTxDataQ, TClsfrParams *pClsfrInitParams);
TI_STATUS txDataClsfr_ClassifyTxPacket (TI_HANDLE hTxDataQ, TTxCtrlBlk *pPktCtrlBlk, TI_UINT8 uPacketDtag);
TI_STATUS txDataClsfr_InsertClsfrEntry (TI_HANDLE hTxDataQ, TClsfrTableEntry *pNewEntry);
TI_STATUS txDataClsfr_RemoveClsfrEntry (TI_HANDLE hTxDataQ, TClsfrTableEntry *pRemEntry);
TI_STATUS txDataClsfr_SetClsfrType     (TI_HANDLE hTxDataQ, EClsfrType eNewClsfrType);
TI_STATUS txDataClsfr_GetClsfrType     (TI_HANDLE hTxDataQ, EClsfrType *pClsfrType);

#ifdef TI_DBG
void      txDataClsfr_PrintClsfrTable  (TI_HANDLE hTxDataQ);
#endif /* TI_DBG */

#endif /* _TX_DATA_QUEUE_API_H_ */

