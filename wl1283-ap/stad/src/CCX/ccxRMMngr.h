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
/*																			*/
/*	  MODULE:	ccxRMMngr.h													*/
/*    PURPOSE:	ccx v2 Radio Measurement module interface header file		*/
/*																			*/
/***************************************************************************/


#ifndef __CCXRMMNGR_H__
#define __CCXRMMNGR_H__
#ifdef CCX_MODULE_INCLUDED





#include "ccxMngr.h"
#include "ccxRMMngrParam.h"
#include "measurementMgr.h"
#include "requestHandler.h"







TI_STATUS measurementMgr_ccxParse(TI_HANDLE hMeasurementMgr, TI_UINT8 *iappPacket);

TI_STATUS measurementMgr_ccxParseFrameReq(TI_HANDLE hMeasurementMgr, 
                                        TI_UINT8 *pData, TI_INT32 dataLen,
                                        TMeasurementFrameRequest *frameReq);

TI_STATUS measurementMgr_ccxParseRequestIEHdr(TI_UINT8 *pData, TI_UINT16 *reqestHdrLen,
                                            TI_UINT16 *measurementToken);

TI_BOOL      measurementMgr_ccxIsTypeValid(TI_HANDLE hMeasurementMgr, 
                                           EMeasurementType type, 
                                           EMeasurementScanMode scanMode);

TI_STATUS measurementMgr_ccxBuildRejectReport(TI_HANDLE hMeasurementMgr,
									MeasurementRequest_t* pRequestArr[],
									TI_UINT8		numOfRequestsInParallel,
									EMeasurementRejectReason	rejectReason);

TI_STATUS measurementMgr_ccxBuildReport(TI_HANDLE hMeasurementMgr, MeasurementRequest_t request, TMeasurementTypeReply * reply);

TI_STATUS measurementMgr_ccxSendReportAndCleanObject(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_radioMngtCapabilityBuild(TI_HANDLE hMeasurementMgr,
											   TI_UINT8 *pRMStateIe,
											   TI_UINT8 *pRMStateIeLen);

#endif /* CCX_MODULE_INCLUDED*/
#endif /* __CCXRMMNGR_H__ */

