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
/*																								*/
/*		MODULE:		ccxRMMngr.c																	*/
/*		PURPOSE:	ccx v2 Radio Measurement module interface.									*/
/*                  This sub-module parser received CCX v2 Radio Measurements from the AP,		*/
/*					performs the requested measurements according to AP command and consrtucts	*/
/*					measurements report. The object responsibles for all ccx v2 RM features		*/
/*					implementation. This object is an internal object of the Measurement object.*/
/*																						 		*/
/************************************************************************************************/


#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_12
#include "ccxRMMngr.h"
#include "measurementMgrApi.h"
#include "siteHash.h"
#include "siteMgrApi.h"
#include "sme.h"



#define     IE_HDR_LEN                          2

#define     RM_REQUEST_HDR_LEN                  4
#define     RM_REQUEST_IE_HDR_LEN               6
#define     RM_REQUEST_IE_LEN                   8

#define		REPORT_IE_HDR_LEN				    4
#define		REPORT_IE_FIXED_FIELDS_LEN			4
#define		REPORT_FRAME_FIXED_FIELDS_LEN	    18

#define		RADIO_MNGT_CAPABILITY_IE_LEN		6
#define		RADIO_MNGT_CAPABILITY_IE_HDR_LEN    2

#define     BEACON_REPORT_FIXED_FIELDS_LEN      28
#define     CHANNEL_LOAD_REPORT_LEN             5
#define     NOISE_HISTOGRAM_REPORT_LEN          12







/********************************************************************************/
/*						Internal Structures.        							*/
/********************************************************************************/

typedef struct
{
    TI_UINT16  dialogToken;
    TI_UINT8   activatioDelay;
    TI_UINT8   measurementOffset;
} RMFrameReqHdr_t;



typedef struct
{
    TI_UINT16  IeId;
    TI_UINT16  Length;
    TI_UINT16  Token;
} RMReqIEHdr_t;






/********************************************************************************/
/*						Internal Functions Prototypes     						*/
/********************************************************************************/

static TI_STATUS measurementMgr_ccxBuildNoiseHistogramReport(TI_HANDLE hMeasurementMgr,
										MeasurementRequest_t request,
										TMeasurementTypeReply * reply);

static TI_STATUS measurementMgr_ccxBuildBeaconReport(TI_HANDLE hMeasurementMgr, 
												MeasurementRequest_t request,
												TMeasurementTypeReply * reply);

static TI_STATUS measurementMgr_ccxBuildChannelLoadReport(TI_HANDLE hMeasurementMgr, 
										MeasurementRequest_t request,
										TMeasurementTypeReply * reply);

static void measurementMgr_ccxBuildReportIEHdr(measurementMgr_t * pMeasurementMgr, MeasurementRequest_t request, 
                             TI_UINT8 reportLen, RM_report_IE_hdr_t * reportIEhdr);









/********************************************************************************/
/*						Interface Functions Implementation 						*/
/********************************************************************************/


/**
 * Parses the given IAPP packet and submits it to the Measurement Manager.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param iappPacket The IAPP packet to parse.
 * 
 * @return TI_OK in case of success, TI_NOK otherwise
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxParse(TI_HANDLE hMeasurementMgr, TI_UINT8 * iappPacket)
{
	measurementMgr_t	  *pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
	TI_UINT8			  *pData;
	IAPP_msg_header_t	  *pckHdr;
	EMeasurementFrameType  frameType;
	TI_INT32			   dataLen;

	/* Checking if measurement is enabled */
	if (pMeasurementMgr->Mode == MSR_MODE_NONE)
		return TI_OK;

	/* Get the IAPP packet header */
	pckHdr = (IAPP_msg_header_t *) iappPacket;

	/* Make sure the subtype of the packet is RM request (= 0x01) */
	if (pckHdr->funcCode != RM_REQUEST)
	{
	    TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_ERROR, ": Frame subType is invalid.\n");

		return TI_NOK;
	}

	/* Find the packet type */
	if (MAC_BROADCAST (pckHdr->destinationMacAddress))
	{
		frameType = MSR_FRAME_TYPE_BROADCAST;
	}
	else
	{
		if (MAC_MULTICAST (pckHdr->destinationMacAddress))
		{
			frameType = MSR_FRAME_TYPE_MULTICAST;
		}
		else
		{
			frameType = MSR_FRAME_TYPE_UNICAST;
		}
	}

	/* Get the packet body and its length */
	pData = iappPacket + IAPP_SNAP_HDR_LEN + IAPP_HDR_LEN;
	dataLen = HTOWLANS(pckHdr->length) - IAPP_HDR_LEN;

    TRACE25(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ":\n\nReceived Frame Hdr:\n SNAP Header:%x %x %x %x %x %x %x %x\nLength: %d\nType: %x, Sub-Type: %x\nDest. Mac Address (Zeros): %x %x %x %x %x %x\nSource Mac Address (Zeros): %x %x %x %x %x %x\n\nFrame Type: %d\nDataLen = %d\n\n",					 pckHdr->snapHeader[0], pckHdr->snapHeader[1], pckHdr->snapHeader[2], pckHdr->snapHeader[3],					 pckHdr->snapHeader[4], pckHdr->snapHeader[5], pckHdr->snapHeader[6], pckHdr->snapHeader[7],					 HTOWLANS(pckHdr->length), pckHdr->messageType, pckHdr->funcCode, 					 pckHdr->destinationMacAddress[0], pckHdr->destinationMacAddress[1], 					 pckHdr->destinationMacAddress[2], pckHdr->destinationMacAddress[3], 					 pckHdr->destinationMacAddress[4], pckHdr->destinationMacAddress[5], 					 pckHdr->sourceMacAddress[0], pckHdr->sourceMacAddress[1], 					 pckHdr->sourceMacAddress[2], pckHdr->sourceMacAddress[3], 					 pckHdr->sourceMacAddress[4], pckHdr->sourceMacAddress[5], 					 frameType, dataLen);

	/* Send parsed packet to measurement object */
	measurementMgr_receiveFrameRequest(hMeasurementMgr, frameType, dataLen, pData);

	return TI_OK;
}




/**
 * Parses the received frame request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param pData The frame request.
 * @param dataLen The length of the frame request.
 * @param frameReq A pointer to a frame request struct which will hold the request.
 * 
 * @return always TI_OK
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxParseFrameReq(TI_HANDLE hMeasurementMgr, 
										  TI_UINT8 * pData, TI_INT32 dataLen,
										  TMeasurementFrameRequest * frameReq)
{
    RMFrameReqHdr_t * RMFrameReqHdr;

    RMFrameReqHdr = (RMFrameReqHdr_t *) pData;
    RMFrameReqHdr->dialogToken = ENDIAN_HANDLE_WORD(RMFrameReqHdr->dialogToken);
    
    frameReq->hdr = (TMeasurementFrameHdr *) RMFrameReqHdr;
    frameReq->requests = pData + RM_REQUEST_HDR_LEN;
    frameReq->requestsLen = dataLen - RM_REQUEST_HDR_LEN;

    return TI_OK;
}




/**
 * Parses the received request IE header.
 * 
 * @param pData The request IE header.
 * @param reqestHdrLen A pointer to an integer which will hold the length of the header.
 * @param measurementToken A pointer to an integer which will hold the measurement token.
 * 
 * @return TI_OK in case of success, TI_NOK otherwise
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxParseRequestIEHdr(TI_UINT8 * pData, TI_UINT16 * reqestHdrLen,
												TI_UINT16 * measurementToken)
{
    RMReqIEHdr_t * RMReqIEHdr = (RMReqIEHdr_t *) pData;
    
    RMReqIEHdr->IeId = ENDIAN_HANDLE_WORD(RMReqIEHdr->IeId);
    /* checking if received the correct information element ID */
    if (RMReqIEHdr->IeId != RM_REQUEST_IE_ID)
        return TI_NOK;
    
    RMReqIEHdr->Length = ENDIAN_HANDLE_WORD(RMReqIEHdr->Length);
    /* check that len is valid */
    if(RMReqIEHdr->Length != RM_REQUEST_IE_LEN)
        return TI_NOK;
    
    *measurementToken = ENDIAN_HANDLE_WORD(RMReqIEHdr->Token);
    
    *reqestHdrLen = RM_REQUEST_IE_HDR_LEN;
    
    return TI_OK;
}




/**
 * Checks if the measurement type is valid.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param type The measurement type.
 * @param scanMode The scan mode.
 * 
 * @return TI_TRUE if valid.
 * 
 * @date 10-Jan-2006
 */
TI_BOOL measurementMgr_ccxIsTypeValid(TI_HANDLE hMeasurementMgr, 
									  EMeasurementType type, 
									  EMeasurementScanMode scanMode)
{
    if (type == MSR_TYPE_FRAME_MEASUREMENT)
    {
        return TI_FALSE;
    }
    else
    {
        return TI_TRUE;
    }
}




/**
 * Builds the Radio Management Capability IE.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param pRMStateIe A pointer to a buffer to hold the IE.
 * @param pRMStateIeLen The length of the IE.
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_radioMngtCapabilityBuild(TI_HANDLE hMeasurementMgr,
												TI_UINT8 * pRMStateIe,
												TI_UINT8 * pRMStateIeLen)
{
	measurementMgr_t * pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
	ccx_radioManagmentCapability_IE_t * pRadioMngtCap_IE;
    TI_UINT8 ccxOUI[3] = CCX_OUI;


	if (pMeasurementMgr->Mode == MSR_MODE_CCX)
	{
        TI_UINT16 rmState = RM_STATE_NORMAL;

        pRadioMngtCap_IE = (ccx_radioManagmentCapability_IE_t *) pRMStateIe;

        pRadioMngtCap_IE->eleID = RADIO_MNGT_CAPABILITY_IE_ID;
        pRadioMngtCap_IE->len   = RADIO_MNGT_CAPABILITY_IE_LEN;
        os_memoryCopy(pMeasurementMgr->hOs, &(pRadioMngtCap_IE->ciscoAironetOUI), &(ccxOUI), 3);
        pRadioMngtCap_IE->version = 1;
        COPY_WLAN_WORD(&(pRadioMngtCap_IE->rmState), &(rmState));
		*pRMStateIeLen = RADIO_MNGT_CAPABILITY_IE_HDR_LEN + RADIO_MNGT_CAPABILITY_IE_LEN;
	}
	else
	{
		*pRMStateIeLen = 0;
	}

	return TI_OK;
}




/**
 * Sends the report to the CCXMngr and cleans up the Measurement Manager object.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxSendReportAndCleanObject(TI_HANDLE hMeasurementMgr)
{
	measurementMgr_t    *pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
	TI_STATUS           status;
    TI_UINT8            pckHdr[24];
    TI_UINT8            *pckBody;
    TI_UINT32           pckLen;
    TI_UINT16           length = REPORT_FRAME_FIXED_FIELDS_LEN;

	/* Building the Report Frame Header */
	IAPP_msg_header_t	reportPckHdr = {IAPP_PACKET_SNAP, 
                                        REPORT_FRAME_FIXED_FIELDS_LEN, 
										IAPP_RADIO_MEASUREMENT, 
										RM_REPORT,
										DUMMY_MAC_ADDRESS,
										DUMMY_MAC_ADDRESS};

	if (pMeasurementMgr->frameLength != 0)
	{
        TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Sending measurement report (frameLength = %d)\n", pMeasurementMgr->frameLength);

		length += pMeasurementMgr->frameLength;
        pckLen = (TI_UINT32)length;
        length = HTOWLANS(length);
        COPY_WLAN_WORD(&(reportPckHdr.length), &length);

        os_memoryZero(pMeasurementMgr->hOs, &pckHdr, sizeof(IAPP_msg_header_t));
        os_memoryCopy(pMeasurementMgr->hOs, &pckHdr, &reportPckHdr, sizeof(IAPP_msg_header_t));

        pckBody = (TI_UINT8 *) (&(pMeasurementMgr->ccxFrameReport));

		/* Sending the Report frame to the ccxMngr in order for it to send it */
		status = ccxMngr_sendIAPPPacket(pMeasurementMgr->hCcxMngr, &reportPckHdr, 
							(TI_UINT8 *) &pMeasurementMgr->ccxFrameReport, pckLen + WLAN_SNAP_HDR_LEN);

		if (status != TI_OK)
			return status;

		/* clearing reports data base */
		os_memoryZero(pMeasurementMgr->hOs, &(pMeasurementMgr->ccxFrameReport),
					  sizeof(RM_report_frame_t));
		pMeasurementMgr->frameLength = 0;
		pMeasurementMgr->nextEmptySpaceInReport = 0;
	}
	else
	{
TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Not sending measurement report (frameLength = %d)\n", pMeasurementMgr->frameLength);
	}

	/* Reset the Measurement Object Params */
	pMeasurementMgr->currentFrameType = MSR_FRAME_TYPE_NO_ACTIVE;
	requestHandler_clearRequests(pMeasurementMgr->hRequestH);

	return TI_OK;

}




/**
 * Build a reject report for the given measurement request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param pRequestArr The measurement request.
 * @param numOfRequestsInParallel The number of parallel measurement requests.
 * @param rejectReason
 * 
 * @return always TI_OK
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxBuildRejectReport (TI_HANDLE hMeasurementMgr,
                                               MeasurementRequest_t * pRequestArr[],
                                               TI_UINT8 numOfRequestsInParallel,
                                               EMeasurementRejectReason rejectReason)
{
	measurementMgr_t    *pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
	RM_report_IE_hdr_t  *reportIE = (RM_report_IE_hdr_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);
	TI_UINT8            measurementMode = 0;
	TI_UINT8            index;
    TI_UINT16           elementID = RM_REPORT_IE_ID;
    TI_UINT16           length    = REPORT_IE_FIXED_FIELDS_LEN;

	/* Set frame dialog token */
	pMeasurementMgr->ccxFrameReport.dialogToken = pRequestArr[0]->frameToken;

	/* Building Report IEs */
	for (index = 0; index < numOfRequestsInParallel; index++)
	{
        COPY_WLAN_WORD(&(reportIE->elementID), &elementID);
        COPY_WLAN_WORD(&(reportIE->length), &length);
        COPY_WLAN_WORD(&(reportIE->token), &(pRequestArr[index]->measurementToken));
		reportIE->type = (TI_UINT8)pRequestArr[index]->Type;
		
		/* setting Parallel Bit */
		if (pRequestArr[index]->isParallel)
			measurementMode |= 0x1;
		
		switch (rejectReason)
		{
			case MSR_REJECT_DTIM_OVERLAP:
			case MSR_REJECT_DURATION_EXCEED_MAX_DURATION:
			case MSR_REJECT_TRAFFIC_INTENSITY_TOO_HIGH:
			case MSR_REJECT_SCR_UNAVAILABLE:
			case MSR_REJECT_INVALID_CHANNEL:
			case MSR_REJECT_NOISE_HIST_FAIL:
			case MSR_REJECT_CHANNEL_LOAD_FAIL:
			case MSR_REJECT_EMPTY_REPORT:
			case MSR_REJECT_MAX_DELAY_PASSED:
			{
				/* Setting the Refused bit */
				measurementMode |= 0x4;
				break;
			}

			case MSR_REJECT_INVALID_MEASUREMENT_TYPE:
			{
				/* Setting the Incapable bit */
				measurementMode |= 0x2;
				break;
			}
							  
			default:
			{
TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_ERROR, ": Measurement #%d rejected with unknown reason (%d)\n", index+1, rejectReason);
				break;
			}
		}
		
		reportIE->mode = measurementMode;

		/* Updating the Length of the Frame */
		pMeasurementMgr->frameLength += REPORT_IE_FIXED_FIELDS_LEN + REPORT_IE_HDR_LEN;

TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_WARNING, ": Measurement #%d rejected with reason %d\n", index+1, rejectReason);
		
		/* Update Frame Report Body Length */
		pMeasurementMgr->nextEmptySpaceInReport += REPORT_IE_HDR_LEN + REPORT_IE_FIXED_FIELDS_LEN;
		reportIE = (RM_report_IE_hdr_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);	

		/* Reset local variables for next IE */
		measurementMode = 0;
	}

	return TI_OK;
}




/**
 * Builds a report for the given measurement request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param request The measurement request.
 * @param reply The reply received from the MeasurementSrv.
 * 
 * @return always TI_OK 
 * 
 * @date 10-Jan-2006
 */
TI_STATUS measurementMgr_ccxBuildReport(TI_HANDLE hMeasurementMgr, MeasurementRequest_t request, TMeasurementTypeReply * reply)
{
	measurementMgr_t * pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
	TI_STATUS status = TI_OK;
	
	/* Getting measurement results */
	switch (request.Type)
	{
		case MSR_TYPE_CCA_LOAD_MEASUREMENT:
		{
TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Building Channel Load report...\n");

			status = measurementMgr_ccxBuildChannelLoadReport(hMeasurementMgr, request, reply);
			break;
		}
		
		case MSR_TYPE_NOISE_HISTOGRAM_MEASUREMENT:
		{
TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Building Noise Histogram report...\n");

			status = measurementMgr_ccxBuildNoiseHistogramReport(hMeasurementMgr, request, reply);
			break;
		}
		
		case MSR_TYPE_BEACON_MEASUREMENT:
		{      
TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Building Beacon report...\n");

			status = measurementMgr_ccxBuildBeaconReport(hMeasurementMgr, request, reply);
			
			break;
		}
		
		case MSR_TYPE_FRAME_MEASUREMENT:
		{
			/* NOT SUPPORTED */
TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_WARNING, ": Frame measurement not supported!\n");

			break;
		}
		
		default:
TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_ERROR, ": Measurement type not recognized (%d)!\n", request.Type);

			break;
	}

	return status;
}




/**
 * Build a report for a Beacon measurement request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param request The measurement request.
 * 
 * @return always TI_OK
 * 
 * @date 10-Jan-2006
 */
static TI_STATUS measurementMgr_ccxBuildBeaconReport(TI_HANDLE hMeasurementMgr, 
												MeasurementRequest_t request,
												TMeasurementTypeReply * reply)
 
{
    measurementMgr_t   *pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;
    beacon_report_t	   *reportIE = (beacon_report_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]); 
    paramInfo_t        param;
    TI_UINT8           ieLen;
    TI_UINT8           reportSize		= 0;
    TI_UINT8           ccxOUI[3]       = CCX_OUI;
    TI_UINT8           ccxOUIAndType[4];
    TI_UINT8		   reportIEIndex	= 0;
    beacon_report_t	   *reportIEArray[50];			/* MAX_RM_REPORT_SIZE / minimum size of beacon_report_t = 50 */	
    TI_UINT32		   addSiteToReport;
    TSiteEntry         *pCurrentSite;
    TI_HANDLE          hScanResultTable;
    

    os_memoryCopy(pMeasurementMgr->hOs, &(ccxOUIAndType[0]), &(ccxOUI[0]), 3);
    ccxOUIAndType[3] = 1;	/* Radio Mngmnt Cap. IE type */

	if ((request.ScanMode != MSR_SCAN_MODE_BEACON_TABLE) && (reply->status != TI_OK))
	{
	    MeasurementRequest_t * reqArr[1];
        reqArr[0] = &request;

        TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Beacon measurement request failed!\n");
        
        measurementMgr_ccxBuildRejectReport(hMeasurementMgr, reqArr, 1, MSR_REJECT_OTHER_REASON);

		return TI_OK;
	}

    sme_GetSmeScanResultTableHandler (pMeasurementMgr->hSme, &hScanResultTable);
    
    /* get the first site from the scan result table */
    pCurrentSite = scanResultTable_GetFirst (hScanResultTable);

    /* Get the relevant site entries from the current site table */
    /* check all sites */
    while (NULL != pCurrentSite)
    {
		siteEntry_t * pSite;
         
        pSite = (siteEntry_t *)pCurrentSite;

        if (pSite->siteType != SITE_NULL)
        {   
            /* 
             * If the site is on the requested channel when the measurement operation was in action, or the request is
			 * of Beacon Table Type --> build the report
             */
            if ((pSite->channel == request.channelNumber) || (request.ScanMode == MSR_SCAN_MODE_BEACON_TABLE))
            {   
				TI_UINT8 reportLength = BEACON_REPORT_FIXED_FIELDS_LEN;
				TI_UINT8 infoElementsLen = 0;
                TI_UINT16 duratonTime = 0;

                addSiteToReport = TI_TRUE;
				reportSize++;

                /* Update the site's channel */
                reportIE->channelNum = pSite->channel; 
                reportIE->spare = 0x00;

                if (request.ScanMode != MSR_SCAN_MODE_BEACON_TABLE)
                {
                    duratonTime = request.DurationTime;
                }
                COPY_WLAN_WORD(&reportIE->duratonTime, &duratonTime);

TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Site found -\n");

TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tChannel = %d, DurationTime = %d\n", 								pSite->channel, reportIE->duratonTime);
                
                /* PHY Type */
                switch (pSite->rxRate)
                {
					case DRV_RATE_1M:
					case DRV_RATE_2M:
						reportIE->phyType = PHY_DSS;
						break;

					case DRV_RATE_5_5M:
					case DRV_RATE_11M:
					case DRV_RATE_22M:
						reportIE->phyType = PHY_HIGH_RATE_DSS;
						break;

					case DRV_RATE_6M:
					case DRV_RATE_9M:
					case DRV_RATE_12M:
					case DRV_RATE_18M:
					case DRV_RATE_24M:
					case DRV_RATE_36M:
					case DRV_RATE_48M:
					case DRV_RATE_54M:
						reportIE->phyType = PHY_OFDM;
						break;

					default: 
						reportIE->phyType = PHY_UNKNOWN;
						break;
                }

                TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tPHY Type = %d, Rx Rate = %d\n", 								reportIE->phyType, pSite->rxRate);

                
                /* RSSI */
                reportIE->recvSignalPower = (TI_INT8) pSite->rssi;

                TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tRSSI = %d (0x%x)\n", pSite->rssi, pSite->rssi);
                

                /* BSSID */
                MAC_COPY (reportIE->bssid, pSite->bssid);
                
                TRACE6(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tBSSID = %x %x %x %x %x %x\n", 								pSite->bssid[0], pSite->bssid[1], pSite->bssid[2], 								pSite->bssid[3], pSite->bssid[4], pSite->bssid[5]);


                /* Parent TSF */
                param.paramType = SITE_MGR_CURRENT_TSF_TIME_STAMP;
                siteMgr_getParam(pMeasurementMgr->hSiteMgr, &param);

                os_memoryCopy(pMeasurementMgr->hOs, reportIE->parentTSF, 
                    param.content.siteMgrCurrentTsfTimeStamp, PARENT_TSF_LEN);

                TRACE4(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tParent TSF = %x %x %x %x\n", 								param.content.siteMgrCurrentTsfTimeStamp[0],								param.content.siteMgrCurrentTsfTimeStamp[1], 								param.content.siteMgrCurrentTsfTimeStamp[2], 								param.content.siteMgrCurrentTsfTimeStamp[3]);
                

                /* Target TSF */
                os_memoryCopy(pMeasurementMgr->hOs, reportIE->targetTSF, 
                    pSite->tsfTimeStamp, TARGET_TSF_LEN);
                
                TRACE8(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tTarget TSF = %x %x %x %x %x %x %x %x\n", 								pSite->tsfTimeStamp[0], pSite->tsfTimeStamp[1], pSite->tsfTimeStamp[2], 								pSite->tsfTimeStamp[3], pSite->tsfTimeStamp[4], pSite->tsfTimeStamp[5], 								pSite->tsfTimeStamp[6], pSite->tsfTimeStamp[7]);


                /* Beacon Interval */
                COPY_WLAN_WORD(&reportIE->beaconInerval, &pSite->beaconInterval);

                TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tBeacon Interval = %d (0x%x)\n", pSite->beaconInterval, pSite->beaconInterval);

                /* Capability Information */
                COPY_WLAN_WORD(&reportIE->capabilities, &pSite->capabilities);

                TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tCapability Information = %x\n", pSite->capabilities);
                
				param.paramType = SITE_MGR_CURRENT_BSSID_PARAM;
				siteMgr_getParam(pMeasurementMgr->hSiteMgr,&param);

                if (!MAC_EQUAL (param.content.siteMgrDesiredBSSID, pSite->bssid))
                {
					TI_UINT8 ieIDIndex;
					TI_UINT8 * desiredIe;
					TI_BOOL foundIe;

                    /* Information Elements */
                    for (ieIDIndex = SSID_IE_ID; ieIDIndex <= IBSS_PARAMETER_SET_IE_ID; ieIDIndex++)
                    {
                        /* getting the IE with the 'ieIDIndex' ID */
                        if (pSite->beaconRecv)
                            foundIe = mlmeParser_ParseIeBuffer (pMeasurementMgr->hMlme, 
                                                                pSite->beaconBuffer, 
                                                                pSite->beaconLength, 
                                                                ieIDIndex, 
                                                                &desiredIe, 
                                                                NULL, 
                                                                0);
                        else
                            foundIe = mlmeParser_ParseIeBuffer (pMeasurementMgr->hMlme, 
                                                                pSite->probeRespBuffer, 
                                                                pSite->probeRespLength, 
                                                                ieIDIndex, 
                                                                &desiredIe, 
                                                                NULL, 
                                                                0);
                        
                        /* Updating the right fields in the report */
                        if (foundIe)
                        {
                            ieLen = desiredIe[1] + IE_HDR_LEN;
                            
                            TRACE2(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tIE ID = %d, Len = %d\n", ieIDIndex, ieLen);
                            
                            os_memoryCopy(pMeasurementMgr->hOs, 
									&reportIE->pInfoElements[infoElementsLen], desiredIe, ieLen);

                            infoElementsLen += ieLen;
                            reportLength += ieLen;
                        }
                    }
                    
                    /* Radio Mngt Capability IE */
                    if (pSite->beaconRecv)
                        foundIe = mlmeParser_ParseIeBuffer (pMeasurementMgr->hMlme, 
                                                            pSite->beaconBuffer, 
                                                            pSite->beaconLength, 
                                                            RADIO_MNGT_CAPABILITY_IE_ID, 
                                                            &desiredIe, 
                                                            ccxOUIAndType, 
                                                            4);
                    else
                        foundIe = mlmeParser_ParseIeBuffer (pMeasurementMgr->hMlme, 
                                                            pSite->probeRespBuffer, 
                                                            pSite->probeRespLength, 
                                                            RADIO_MNGT_CAPABILITY_IE_ID, 
                                                            &desiredIe, ccxOUIAndType, 
                                                            4);
                    
                    if (foundIe)
                    {
						TI_UINT8  mbssidMask;
                        TI_UINT16 rmState;

                        /* get the MBSSID MASK value from adio Mngt Capability IE */
                        COPY_WLAN_WORD(&rmState, &(((ccx_radioManagmentCapability_IE_t *)desiredIe)->rmState));
                        mbssidMask = (TI_UINT8)(HTOWLANS(rmState) & RADIO_MNGT_MBSSID_MASK_MASK);

                        ieLen = desiredIe[1] + IE_HDR_LEN;
                        
						/* Acording to CCX4: Only a single BSSID falling in the range of BSSIDs defined by the MBSSID 
						mask shall be reported in the Beacon Report. */
						if (mbssidMask)
   						{
   							int i;
   							TI_UINT8 mask = 1;

   							/* convert value of MBSSID MASK to mask of bits */
							for (--mbssidMask ;mbssidMask > 0; --mbssidMask ) 
   							{
   								mask = mask << 1;
   								mask |=1;
   							}

   							/* preform the checking on the BSSID that alrady in the report */
							for(i = 0; i < reportIEIndex; ++i)
								/* verify that the 5 bytes of the BSSID are equaled */
   								if( ! os_memoryCompare(pMeasurementMgr->hOs, reportIE->bssid, reportIEArray[i]->bssid, MAC_ADDR_LEN - 1))
									/* mask the last LSB with the mask and verify if they equaled */  
									if((reportIE->bssid[5] & ~mask) == (reportIEArray[i]->bssid[5] & ~mask))
									{
                        TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ":\nAnother BSSIDs defined as part of MBSSID. Will not be add to the beacon report!\n", ieLen);

										/* will not be add to the report */
										addSiteToReport = TI_FALSE;
									}
						}
                        
                        os_memoryCopy(pMeasurementMgr->hOs, 
									  &reportIE->pInfoElements[infoElementsLen], 
									  desiredIe, ieLen);

                        infoElementsLen += ieLen;
                        reportLength += ieLen;
                    }
                }
				else
				{
                        TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": \t\tCurrent site encountered.\n");
				}
                
                if(addSiteToReport)
				{
					/* Save just the pointers of the BSSID that added to the report */
					reportIEArray[reportIEIndex] = (beacon_report_t*)&(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);
					reportIEIndex++;

                /* Build Report IE Header */
                measurementMgr_ccxBuildReportIEHdr(pMeasurementMgr, request, reportLength, &(reportIE->hdr));
				}
                
                /* Preparing for the next site */
                reportIE = (beacon_report_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);
                infoElementsLen = 0;
            } /* if same channel and detected while measuring */
     
        } /* if site is valid */

        /* and continue to the next site */
        pCurrentSite = scanResultTable_GetNext (hScanResultTable);

    } /* while */

TRACE1(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Number of sites in the report = %d\n", reportSize);

    if (reportSize <= 0)
    {
		MeasurementRequest_t * reqArr[1];
        reqArr[0] = &request;
        measurementMgr_ccxBuildRejectReport(hMeasurementMgr, reqArr, 1, MSR_REJECT_EMPTY_REPORT);
    }
    
    return TI_OK;
}





/**
 * Builds a report for a Channel Load measurement request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param request The measurement request.
 * @param reply The reply received from the MeasurementSrv.
 * 
 * @return always TI_OK 
 * 
 * @date 10-Jan-2006
 */
static TI_STATUS measurementMgr_ccxBuildChannelLoadReport(TI_HANDLE hMeasurementMgr, 
										MeasurementRequest_t request,
										TMeasurementTypeReply * reply)
{
    measurementMgr_t * pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;


    if (reply->status == TI_OK)
    {
	    channelLoad_report_t * pChannelLoadReport = (channelLoad_report_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);

        pChannelLoadReport->channelNum		= request.channelNumber;
        pChannelLoadReport->spare			= 0x00;

        COPY_WLAN_WORD(&pChannelLoadReport->duratonTime, &request.DurationTime);

		pChannelLoadReport->ccaBusyFraction = reply->replyValue.CCABusyFraction;
        
        TRACE3(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Channel = %d, durationTime = %d, ccaBusyFraction = %d\n",						pChannelLoadReport->channelNum, request.DurationTime, pChannelLoadReport->ccaBusyFraction);
        
        measurementMgr_ccxBuildReportIEHdr(pMeasurementMgr, request, CHANNEL_LOAD_REPORT_LEN, 
            &(pChannelLoadReport->hdr));
    }
    else
    {
	    MeasurementRequest_t * reqArr[1];
        reqArr[0] = &request;

        TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": channel Load measurement request failed!\n");
        
        measurementMgr_ccxBuildRejectReport(hMeasurementMgr, reqArr, 1, MSR_REJECT_CHANNEL_LOAD_FAIL);
    }

	return TI_OK;
}




/**
 * Builds a report for a Noise Histogram measurement request.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param request The measurement request.
 * @param reply The reply received from the MeasurementSrv.
 * 
 * @return always TI_OK 
 * 
 * @date 10-Jan-2006
 */
static TI_STATUS measurementMgr_ccxBuildNoiseHistogramReport(TI_HANDLE hMeasurementMgr,
										MeasurementRequest_t request,
										TMeasurementTypeReply * reply)
{
    measurementMgr_t			* pMeasurementMgr = (measurementMgr_t *) hMeasurementMgr;


    if (reply->status == TI_OK)
    {
		noiseHistogram_report_t * pNoiseHistogramReport = (noiseHistogram_report_t *) &(pMeasurementMgr->ccxFrameReport.reportElements[pMeasurementMgr->nextEmptySpaceInReport]);
		TI_UINT8  index;

        pNoiseHistogramReport->channelNum	= request.channelNumber;
        pNoiseHistogramReport->spare		= 0x00;

        COPY_WLAN_WORD(&pNoiseHistogramReport->duratonTime, &request.DurationTime);
        
        for (index = 0; index < NUM_OF_NOISE_HISTOGRAM_COUNTERS; index++)
        {
            pNoiseHistogramReport->rpiDensity[index] = reply->replyValue.RPIDensity[index];
        }
        
        TRACE10(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Channel = %d, durationTime = %d, RPIDensity = %d,%d,%d,%d,%d,%d,%d,%d\n", 						pNoiseHistogramReport->channelNum, pNoiseHistogramReport->duratonTime,						pNoiseHistogramReport->rpiDensity[0], pNoiseHistogramReport->rpiDensity[1],						pNoiseHistogramReport->rpiDensity[2], pNoiseHistogramReport->rpiDensity[3],						pNoiseHistogramReport->rpiDensity[4], pNoiseHistogramReport->rpiDensity[5],						pNoiseHistogramReport->rpiDensity[6], pNoiseHistogramReport->rpiDensity[7]);
      
        measurementMgr_ccxBuildReportIEHdr(pMeasurementMgr, request, 
            NOISE_HISTOGRAM_REPORT_LEN, &(pNoiseHistogramReport->hdr));
    }
    else
    {
		MeasurementRequest_t * reqArr[1];
        reqArr[0] = &request;

        TRACE0(pMeasurementMgr->hReport, REPORT_SEVERITY_INFORMATION, ": Noise Histogram measurement request failed!\n");
        
        measurementMgr_ccxBuildRejectReport(hMeasurementMgr, reqArr, 1, MSR_REJECT_NOISE_HIST_FAIL);
    }

	return TI_OK;
}





/**
 * Build a report IE header.
 * 
 * @param hMeasurementMgr A handle to the Measurement Manager module.
 * @param request The measurement request.
 * @param reportLen	The length of the report.
 * @param reportIEhdr A pointer to the struct which will hold the report.
 * 
 * @date 10-Jan-2006
 */
static void measurementMgr_ccxBuildReportIEHdr(measurementMgr_t * pMeasurementMgr, MeasurementRequest_t request, 
                             TI_UINT8 reportLen, RM_report_IE_hdr_t * reportIEhdr)
{
    TI_UINT8  measurementMode = 0;
    TI_UINT16 elementID = RM_REPORT_IE_ID;
    TI_UINT16 length    = REPORT_IE_FIXED_FIELDS_LEN;

    /* Set frame dialog token */
    COPY_WLAN_WORD(&pMeasurementMgr->ccxFrameReport.dialogToken, &request.frameToken);
	
	/* Building Report IE */
    COPY_WLAN_WORD(&reportIEhdr->elementID, &elementID);
    COPY_WLAN_WORD(&reportIEhdr->token, &request.measurementToken);
	reportIEhdr->type = (TI_UINT8) request.Type;
	
	/* setting Parallel Bit */
	if (request.isParallel)
		measurementMode |= 0x1;
	
	reportIEhdr->mode = measurementMode;
	
	/* Updating the Lengths of the IE and the Frame.*/
	length += reportLen;
    COPY_WLAN_WORD(&reportIEhdr->length, &length);
    pMeasurementMgr->frameLength += length + REPORT_IE_HDR_LEN;
	pMeasurementMgr->nextEmptySpaceInReport += length + REPORT_IE_HDR_LEN;
}


#endif /* CCX_MODULE_INCLUDED */



