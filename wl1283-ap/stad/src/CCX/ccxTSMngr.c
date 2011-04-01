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

/************************************************************************************************/
/*                                                                                              */
/*      MODULE:     ccxTSMngr.c                                                                 */
/*      PURPOSE:    ccx v4 Traffic Stream Measurement module.                           */
/*                  This sub-module parser receives CCX v2 Traffic Stream Measurements from AP, */
/*                  performs the requested measurements according to AP command and consrtucts  */
/*                  measurements report. This object is an internal object of the Measurement   */
/*                                                                                              */
/************************************************************************************************/


#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_13
#include "measurementMgr.h"
#include "ccxTSMngr.h"
#include "ccxMngr.h"
#include "timer.h"
#include "osApi.h"
#include "DataCtrl_Api.h"


typedef struct
{
    TI_UINT16 dialogToken;
    TI_UINT16 eleId;
    TI_UINT16 eleLen;
    TI_UINT16 token;
    TI_UINT8  mode;
    TI_UINT8  type;
    TI_UINT16 averageQDelay;       /* Average Packet Queue Delay */
    TI_UINT16 histogramQDelay[CCX_TSM_HISTOGRAM_TABLE_SIZE];/* Histogram for Packet Queue Delay */
    TI_UINT32 averageTxMediaDelay; /* Average packet transmit media delay */
    TI_UINT16 packetLossCount;     /* Total number of upstream packets dropped */
    TI_UINT16 packetCount;         /* Total number of upstream packets counted */
    TI_UINT8  roamingInfo[3];      /* Note: Use byte array to avoid padding. used as follows:
                                 * TI_UINT8  roamingCount - Total number of successful roams
                                 * TI_UINT16 roamingDelay - The delay of the latest roam 
                                 */
} reportPktBody_t;

extern int WMEQosTagToACTable[MAX_NUM_OF_802_1d_TAGS];
static void measurementMgr_sendTSMReport_AC_BE (TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured);
static void measurementMgr_sendTSMReport_AC_BK (TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured);
static void measurementMgr_sendTSMReport_AC_VI (TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured);
static void measurementMgr_sendTSMReport_AC_VO (TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured);
static void measurementMgr_sendTSMReport (TI_HANDLE hMeasurementMgr, TI_UINT8 acID);



/************************************************************************
 *                        measurementMgr_enableTsMetrics                *
 ************************************************************************
DESCRIPTION: The function is called upon reception of TS metrics IE in 
             ADD TSPEC response or re-associtatee response
                                                                                                   
INPUT:      hMeasurementMgr -   Measurement Manager handle.
            pParams         -   pointer to received IE parameters structure, 
            including enable/disable state and requested interval. 
OUTPUT:     

RETURN:     -

************************************************************************/
void measurementMgr_enableTsMetrics(TI_HANDLE hMeasurementMgr, dot11_TS_METRICS_IE_t *pParams)
{
    measurementMgr_t *pMeasurementMgr = hMeasurementMgr;
    TTxDataCounters updatedTxCounters;
    paramInfo_t param;
    int ac = WMEQosTagToACTable[pParams->tsid];
    TTimerCbFunc fTimerExpiryFunc;

    TRACE4(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, "measurementMgr_enableTsMetrics: interval= %d, ac=%d, curr state=%d, requested=%d\n", TUs_TO_MSECs(WLANTOHS(pParams->measureInterval)), ac, pMeasurementMgr->isTsMetricsEnabled[ac], pParams->state);

    /* Requested to stop or to start metrics - in any case stop the timer */
    tmr_StopTimer (pMeasurementMgr->hTsMetricsReportTimer[ac]);

    if (pParams->state == CCX_TS_METRICS_ENABLED)
    {
        /* Do not clear statistics in case of roaming */
        if (pMeasurementMgr->isTsMetricsEnabled[ac] == TI_FALSE)
        {
            /* Clear (by reading) the statistics */
            TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, "measurementMgr_enableTsMetrics: clearing statistics\n");
            param.paramType = TX_CTRL_REPORT_TS_STATISTICS;
            param.content.tsMetricsCounters.acID = ac;
            param.content.tsMetricsCounters.pTxDataCounters = &updatedTxCounters;
            txCtrlParams_getParam(pMeasurementMgr->hTxCtrl, &param);
            
            apConn_resetRoamingStatistics (pMeasurementMgr->hApConn);
        }

        /* Start periodic TS metrics timer for the requested AC */
        pMeasurementMgr->isTsMetricsEnabled[ac] = TI_TRUE;

        switch (ac) 
        {
		case QOS_AC_BE:  fTimerExpiryFunc = measurementMgr_sendTSMReport_AC_BE;   break;
		case QOS_AC_BK:  fTimerExpiryFunc = measurementMgr_sendTSMReport_AC_BK;   break;
		case QOS_AC_VI:  fTimerExpiryFunc = measurementMgr_sendTSMReport_AC_VI;   break;
		case QOS_AC_VO:  fTimerExpiryFunc = measurementMgr_sendTSMReport_AC_VO;   break;
		default:
			{
TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_FATAL_ERROR, "measurementMgr_enableTsMetrics: clearing statistics\n");
				fTimerExpiryFunc = measurementMgr_sendTSMReport_AC_BE;   
				break;
			}
		}

        tmr_StartTimer (pMeasurementMgr->hTsMetricsReportTimer[ac],
                        fTimerExpiryFunc,
                        (TI_HANDLE)pMeasurementMgr,
                        TUs_TO_MSECs((TI_UINT32)WLANTOHS(pParams->measureInterval)),
                        TI_TRUE);
    }
    else
    {
        pMeasurementMgr->isTsMetricsEnabled[ac] = TI_FALSE;
    }
}


/************************************************************************
 *                        measurementMgr_disableTsMetrics                   *
 ************************************************************************
DESCRIPTION: This function is called when voice call is terminated or 
             when the entire connection is lost. It stops the timer and 
             updates the state to "not measuring".
                                                                                                   
INPUT:      hMeasurementMgr -   Measurement Manager handle.
OUTPUT:     

RETURN:     -

************************************************************************/
void measurementMgr_disableTsMetrics(TI_HANDLE hMeasurementMgr, int ac)
{
    measurementMgr_t *pMeasurementMgr = hMeasurementMgr;
    TI_UINT32 currAC;

    TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, "measurementMgr_disableTsMetrics: ac=%d\n", ac);

    /* Check if requested to stop metrix for specific AC or for all ACs */
    if (ac < MAX_NUM_OF_AC) 
    {
        if (pMeasurementMgr->isTsMetricsEnabled[ac]) 
        {
            tmr_StopTimer (pMeasurementMgr->hTsMetricsReportTimer[ac]);
            pMeasurementMgr->isTsMetricsEnabled[ac] = TI_FALSE;
        }
    }
    else /* Stop all measurings */
    {
        for (currAC = 0; currAC < MAX_NUM_OF_AC; currAC++)
        {
            if (pMeasurementMgr->isTsMetricsEnabled[currAC]) 
            {
                tmr_StopTimer (pMeasurementMgr->hTsMetricsReportTimer[currAC]);
                pMeasurementMgr->isTsMetricsEnabled[currAC] = TI_FALSE;
            }
        }
    }
}


/************************************************************************
 *                        measurementMgr_stopTsMetrics                  *
 ************************************************************************
DESCRIPTION: This function is called when roaming is started. It stops the timers.
                                                                                                   
INPUT:      hMeasurementMgr -   Measurement Manager handle.
OUTPUT:     

RETURN:     -

************************************************************************/
void measurementMgr_stopTsMetrics(TI_HANDLE hMeasurementMgr)
{
    measurementMgr_t *pMeasurementMgr = hMeasurementMgr;
    TI_UINT32 currAC;

    TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, "measurementMgr_stopTsMetrics\n");

    for (currAC = 0; currAC < MAX_NUM_OF_AC; currAC++)
    {
        if (pMeasurementMgr->isTsMetricsEnabled[currAC]) 
        {
            tmr_StopTimer (pMeasurementMgr->hTsMetricsReportTimer[currAC]);
        }
    }
}


/************************************************************************
 *                        measurementMgr_sendTSMReport                  *
 ************************************************************************
DESCRIPTION: This function is called when TSM timeout occurs. 
             It is responsible to gather statistics for the requested 
             access category (in current implementation assumed that it 
             will be voice), and to send report packet to the AP. 
             Since the timer is periodic, it will be automatically restarted. 
             4 callbacks are defined in order to determine the access category;
             all four will eventually call this function
                                                                                                   
INPUT:      pMeasurementMgr -   Measurement Manager handle.
            bTwdInitOccured -   Indicates if TWDriver recovery occured since timer started 
OUTPUT:     

RETURN:     -

************************************************************************/
static void measurementMgr_sendTSMReport_AC_BE(TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured)
{
    measurementMgr_sendTSMReport(hMeasurementMgr, QOS_AC_BE);
}

static void measurementMgr_sendTSMReport_AC_BK(TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured)
{
    measurementMgr_sendTSMReport(hMeasurementMgr, QOS_AC_BK);
}

static void measurementMgr_sendTSMReport_AC_VI(TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured)
{
    measurementMgr_sendTSMReport(hMeasurementMgr, QOS_AC_VI);
}

static void measurementMgr_sendTSMReport_AC_VO(TI_HANDLE hMeasurementMgr, TI_BOOL bTwdInitOccured)
{
    measurementMgr_sendTSMReport(hMeasurementMgr, QOS_AC_VO);
}


static void measurementMgr_sendTSMReport(TI_HANDLE hMeasurementMgr, TI_UINT8 acID)
{
    measurementMgr_t *pMeasurementMgr = (measurementMgr_t *)hMeasurementMgr;
    reportPktBody_t reportPktBody;
    IAPP_msg_header_t   reportPktHdr = {IAPP_PACKET_SNAP, 
                                        0, 
                                        IAPP_RADIO_MEASUREMENT, 
                                        RM_REPORT,
                                        {0, 0, 0, 0, 0, 0},
                                        DUMMY_MAC_ADDRESS};/* Report Frame Header */
    TTxDataCounters updatedTxCounters;
    paramInfo_t param;
    TI_UINT8  roamingCount;
    TI_UINT16 roamingDelay;

    reportPktHdr.length = HTOWLANS(CCX_TS_MEASUREMENT_IE_LEN);  /* IAPP header is BIG endian!! */

    /* Fill in predefined fields and header for the Measurement IE */                       
    reportPktBody.dialogToken =         0;
    reportPktBody.eleId  =  ENDIAN_HANDLE_WORD(IAPP_TS_METRICS_REPORT_IE_ID);     /* Little endian! */
    reportPktBody.eleLen =  ENDIAN_HANDLE_WORD(IAPP_TS_METRICS_REPORT_IE_LENGTH); /* Little endian! */
    reportPktBody.token =   0;
    reportPktBody.mode =    0;
    reportPktBody.type =    6;


    /* Note: The packet body is handled as little endian! */

    /* Gather the info to be reported */
    param.paramType = TX_CTRL_REPORT_TS_STATISTICS;
    param.content.tsMetricsCounters.acID = acID;
    param.content.tsMetricsCounters.pTxDataCounters = &updatedTxCounters;
    txCtrlParams_getParam(pMeasurementMgr->hTxCtrl, &param);

    /* In case there were no packets of measured AC sent, avoid division by zero */
    if (updatedTxCounters.NumPackets == 0) 
    {
        /* Average Packet Queue Delay */
        reportPktBody.averageQDelay = 0;
        /* Average packet transmit media delay */
        reportPktBody.averageTxMediaDelay = 0;

        TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Avg Queue Delay = N/A, Avg Media Delay = N/A\n");
    }
    else
    {
        /* Average Packet Queue Delay */
        reportPktBody.averageQDelay = 
            ENDIAN_HANDLE_WORD((TI_UINT16)(updatedTxCounters.SumTotalDelayMs / updatedTxCounters.NumPackets));
        /* Average packet transmit media delay */
        reportPktBody.averageTxMediaDelay = 
            ENDIAN_HANDLE_LONG((TI_UINT32)(updatedTxCounters.SumFWDelayUs / updatedTxCounters.NumPackets));

   		/* Check If Queue delay is smaller than media delay */
		if ( ((TI_UINT32)(reportPktBody.averageQDelay * 1000)) < (reportPktBody.averageTxMediaDelay) )
		{
			/* Add 1 to make sure that QDelay is larger than MediaDelay, It can happen because of the dividing round off */
			reportPktBody.averageQDelay++;
		}
        TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Avg Queue Delay = %d ms, Avg Media Delay = %d us\n",								 reportPktBody.averageQDelay,								 reportPktBody.averageTxMediaDelay);
    }
    /* Histogram for Packet Queue Delay */
    reportPktBody.histogramQDelay[CCX_TSM_MORE_THAN_0_MS] = 
        ENDIAN_HANDLE_WORD((TI_UINT16)(updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_0_TO_1] +
        updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_1_TO_10]));
    reportPktBody.histogramQDelay[CCX_TSM_MORE_THAN_10_MS] = 
        ENDIAN_HANDLE_WORD((TI_UINT16)updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_10_TO_20]);
    reportPktBody.histogramQDelay[CCX_TSM_MORE_THAN_20_MS] = 
        ENDIAN_HANDLE_WORD((TI_UINT16)updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_20_TO_40]);
    reportPktBody.histogramQDelay[CCX_TSM_MORE_THAN_40_MS] = 
        ENDIAN_HANDLE_WORD((TI_UINT16)(updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_40_TO_60] +
        updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_60_TO_80] +
        updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_80_TO_100] +
        updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_100_TO_200] +
        updatedTxCounters.txDelayHistogram[TX_DELAY_RANGE_ABOVE_200]));
    /* Total number of upstream packets dropped */
    reportPktBody.packetLossCount = ENDIAN_HANDLE_WORD((TI_UINT16)updatedTxCounters.OtherFailCounter);
    /* Get total number of upstream packets counted */
    reportPktBody.packetCount = ENDIAN_HANDLE_WORD((TI_UINT16)updatedTxCounters.NumPackets);

    /* Get roaming statistics */
    apConn_getRoamingStatistics (pMeasurementMgr->hApConn, &roamingCount, &roamingDelay);
    reportPktBody.roamingInfo[0] = roamingCount;
    COPY_WLAN_WORD(&reportPktBody.roamingInfo[1], &roamingDelay);

    /* Sending the Report frame to the ccxMngr */
    ccxMngr_sendIAPPPacket(pMeasurementMgr->hCcxMngr, 
                           &reportPktHdr, 
                           (TI_UINT8 *) &reportPktBody, 
                           CCX_TS_MEASUREMENT_IE_LEN + WLAN_SNAP_HDR_LEN);
}


#endif /* CCX_MODULE_INCLUDED */


