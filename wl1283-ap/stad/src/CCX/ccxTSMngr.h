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
/*	  MODULE:	ccxTSMngr.h													*/
/*    PURPOSE:	ccx v4 Traffic Stream Measurement module header file		*/
/*																			*/
/***************************************************************************/


#ifdef CCX_MODULE_INCLUDED
#ifndef __CCXTSMNGR_H__
#define __CCXTSMNGR_H__


#include "802_11Defs.h"

typedef enum
{
	CCX_TS_METRICS_DISABLED = 0,
	CCX_TS_METRICS_ENABLED = 1
} CCX_TSMetricsState_t;

typedef enum
{
	CCX_TSM_MORE_THAN_0_MS = 0,
	CCX_TSM_MORE_THAN_10_MS = 1,
	CCX_TSM_MORE_THAN_20_MS = 2,
	CCX_TSM_MORE_THAN_40_MS = 3,
	CCX_TSM_HISTOGRAM_TABLE_SIZE
} CCX_TSMHistogramQDelayIndex_t;

#define CCX_TS_MEASUREMENT_IE_LEN	47


void measurementMgr_enableTsMetrics(TI_HANDLE hMeasurementMgr, dot11_TS_METRICS_IE_t *pParams);
void measurementMgr_disableTsMetrics(TI_HANDLE hMeasurementMgr, int ac);
void measurementMgr_stopTsMetrics(TI_HANDLE hMeasurementMgr);

#endif /* __CCXTSMNGR_H__ */
#endif /*  CCX_MODULE_INCLUDED */


