/*
 * ScanCncn.c
 *
 * Copyright(c) 1998 - 2010 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file ScanCncn.c
 *  \brief Scan concentartor implementation
 *
 *  \see   ScanCncnSm.c, ScanCncnSmSpecific.c
 */


#define __FILE_ID__  FILE_ID_76
#include "ScanCncn.h"
#include "report.h"
#include "timer.h"
#include "scrApi.h"
#include "regulatoryDomainApi.h"
#include "siteMgrApi.h"
#include "healthMonitor.h"
#include "TWDriver.h"
#include "DrvMainModules.h"
#include "GenSM.h"
#include "ScanCncnPrivate.h"
#include "ScanCncnOsSm.h"
#include "scanResultTable.h"
#include "smeApi.h"
#include "apConnApi.h"
#include "EvHandler.h"
#include "pwrState.h"



/* static functions */
void scanCncn_SGupdateScanParams (TI_HANDLE hScanCncn, UScanParams *puScanParams, TI_BOOL bPeriodicScan) ;
static TI_STATUS scanCncn_VerifyChannelsWithRegDomain (TI_HANDLE hScanCncn, UScanParams *puScanParams, TI_BOOL bPeriodicScan);
static TI_STATUS scanCncn_PeriodicVerifyChannelsWithRegDomain (TI_HANDLE hScanCncn, TPeriodicScanParams *pPeriodicScanParams);
static void scanCncn_Mix1ShotScanChannels (TScanChannelEntry *pChannelArray, TI_UINT32 uValidChannelsCount);
static void scanCncn_MixPeriodicScanChannels (TPeriodicChannelEntry *pChannelArray, TI_UINT32 uValidChannelsCount);
static void scanCncn_ScanCompleteCB( TI_HANDLE hScanCncn, char* str, TI_UINT32 strLen );

#define SCAN_CLIENT_FROM_TAG( tag )  tag2Client[ tag ];
static EScanCncnClient tag2Client[ SCAN_RESULT_TAG_MAX_NUMBER ] = 
    { SCAN_SCC_NO_CLIENT, SCAN_SCC_APP_ONE_SHOT, SCAN_SCC_DRIVER, SCAN_SCC_APP_PERIODIC, SCAN_SCC_NO_CLIENT,
      SCAN_SCC_ROAMING_IMMED, SCAN_SCC_ROAMING_CONT };

#define DEFAULT_MAX_DWELL_TIME_PASSIVE 105
#define DEFAULT_MIN_DWELL_TIME_PASSIVE 105
/** 
 * \fn     scanCncn_Create 
 * \brief  Create the scan concentrator object
 * 
 * Create the scan concentrator object. Allocates system resources and creates the client modules.
 * 
 * \param  hOS - handle to the OS object
 * \return hanlde to the new scan concentrator object
 * \sa     scanCncn_Destroy, scanCncn_Init, scanCncn_SetDefaults
 */ 
TI_HANDLE scanCncn_Create (TI_HANDLE hOS)
{
    TScanCncn   *pScanCncn;
    TI_UINT32   uIndex;

    /* Allocate scan concentartor object memory */
    pScanCncn = (TScanCncn*)os_memoryAlloc (hOS, sizeof (TScanCncn));
    if (NULL == pScanCncn)
    {
        WLAN_OS_REPORT (("scanCncn_Create: Unable to allocate memory for scan concentrator object\n"));
        return NULL;
    }

    /* nullify the new object */
    os_memorySet (hOS, (void*)pScanCncn, 0, sizeof (TScanCncn));

    /* Store OS handle */
    pScanCncn->hOS = hOS;

    /* Create different clients */
    for (uIndex = 0; uIndex < SCAN_SCC_NUM_OF_CLIENTS; uIndex++)
    {
        pScanCncn->pScanClients[ uIndex ] = scanCncnSm_Create (hOS);
        if (NULL == pScanCncn->pScanClients[ uIndex ])
        {
            WLAN_OS_REPORT (("scanCncn_Create: Unable to create client %d object\n", uIndex));
            /* free all resources allocated so far */
            scanCncn_Destroy ((TI_HANDLE)pScanCncn);
            return NULL;
        }
    }

    /* create the OS scan SM */
    pScanCncn->hOSScanSm = scanCncnOsSm_Create ((TI_HANDLE)pScanCncn);
    if (NULL == pScanCncn->hOSScanSm)
    {
        WLAN_OS_REPORT (("scanCncn_Create: Unable to create OS scan SM\n"));
        /* free all resources allocated so far */
        scanCncn_Destroy ((TI_HANDLE)pScanCncn);
        return NULL;        
    }

    /* create the app scan result table */
    pScanCncn->hScanResultTable = scanResultTable_Create (hOS, SCAN_CNCN_APP_SCAN_TABLE_ENTRIES);
    if (NULL == pScanCncn->hScanResultTable)
    {
        WLAN_OS_REPORT (("scanCncn_Create: Unable to create application scan result table\n"));
        /* free all resources allocated so far */
        scanCncn_Destroy ((TI_HANDLE)pScanCncn);
        return NULL;        
    }

    /* return handle to the new object */
    return (TI_HANDLE)pScanCncn;
}

/** 
 * \fn     scanCncn_Destroy
 * \brief  Destroys the scan concentrator object
 * 
 * Destroys the scan concentrator object. Destroys the cleint modules and frees system resources.
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \return None
 * \sa     scanCncn_Create
 */ 
void scanCncn_Destroy (TI_HANDLE hScanCncn)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;
    TI_UINT32   uIndex;

    /* destory the app scan result table */
    scanResultTable_Destroy (pScanCncn->hScanResultTable);

    /* destroy the OS scan SM */
    scanCncnOsSm_Destroy (hScanCncn);

    /* destroy the client objects */
    for (uIndex = 0; uIndex < SCAN_SCC_NUM_OF_CLIENTS; uIndex++)
    {
        scanCncnSm_Destroy (pScanCncn->pScanClients[ uIndex ]);
    }

    /* release the scan concentrator object */
    os_memoryFree (pScanCncn->hOS, hScanCncn, sizeof (TScanCncn));
}

/** 
 * \fn     scanCncn_Init
 * \brief  Initializes the scan concentartor module 
 * 
 * Copy handles, register callbacks and initialize client modules
 * 
 * \param  pStadHandles - modules handles structure
 * \return None
 * \sa     scanCncn_Create, scanCncn_SetDefaults
 */ 
void scanCncn_Init (TStadHandlesList *pStadHandles)
{
    TScanCncn   *pScanCncn = (TScanCncn*)pStadHandles->hScanCncn;
    
    /* store handles */
    pScanCncn->hTWD = pStadHandles->hTWD;
    pScanCncn->hReport = pStadHandles->hReport;
    pScanCncn->hRegulatoryDomain = pStadHandles->hRegulatoryDomain;
    pScanCncn->hSiteManager = pStadHandles->hSiteMgr;
    pScanCncn->hSCR = pStadHandles->hSCR;
    pScanCncn->hAPConn = pStadHandles->hAPConnection;
    pScanCncn->hEvHandler = pStadHandles->hEvHandler;
    pScanCncn->hMlme = pStadHandles->hMlme;
    pScanCncn->hHealthMonitor = pStadHandles->hHealthMonitor;
    pScanCncn->hSme = pStadHandles->hSme;
	pScanCncn->hTimer = pStadHandles->hTimer;
	pScanCncn->hPwrState = pStadHandles->hPwrState;

    /* nullify other parameters */
    pScanCncn->eConnectionStatus = STA_NOT_CONNECTED;
    pScanCncn->bUseSGParams = TI_FALSE; /* bUseSGParams is TI_TRUE only when SG module is enabled */
    pScanCncn->uOSScanLastTimeStamp = 0;
    pScanCncn->bOSScanRunning = TI_FALSE;
	pScanCncn->numOfConsTimerExpiry = 0;
	pScanCncn->bScanCompleteFlag = TI_TRUE;
    pScanCncn->bPendingPeriodicScan = TI_FALSE;

    /* initialize client objects */
    scanCncnSm_Init (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ], pStadHandles,
                     scanCncnSmImmed1Shot_ScrRequest, scanCncnSmImmed1Shot_ScrRelease, scanCncnSmImmed1Shot_StartScan,
                     scanCncnSmImmed1Shot_StopScan, scanCncnSm_NoOp, "Immediate scan SM", SCAN_RESULT_TAG_IMMEDIATE);
    scanCncnSm_Init (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ], pStadHandles,
                     scanCncnSmCont1Shot_ScrRequest, scanCncnSmCont1Shot_ScrRelease, scanCncnSmCont1Shot_StartScan,
                     scanCncnSmCont1Shot_StopScan, scanCncnSm_NoOp, "Continuous scan SM", SCAN_RESULT_TAG_CONTINUOUS);
    scanCncnSm_Init (pScanCncn->pScanClients[ SCAN_SCC_DRIVER ], pStadHandles,
                     scanCncnSmDrvP_ScrRequest, scanCncnSmDrvP_ScrRelease, scanCncnSmDrvP_StartScan,
                     scanCncnSmDrvP_StopScan, scanCncnSmDrvP_Recovery, "Driver scan SM", SCAN_RESULT_TAG_DRIVER_PERIODIC);
    scanCncnSm_Init (pScanCncn->pScanClients[ SCAN_SCC_APP_PERIODIC ], pStadHandles,
                     scanCncnSmAppP_ScrRequest, scanCncnSmAppP_ScrRelease, scanCncnSmAppP_StartScan,
                     scanCncnSmAppP_StopScan, scanCncnSm_NoOp, "Periodic application scan SM", SCAN_RESULT_TAG_APPLICATION_PEIODIC);
    scanCncnSm_Init (pScanCncn->pScanClients[ SCAN_SCC_APP_ONE_SHOT ], pStadHandles,
                     scanCncnSmApp1Shot_ScrRequest, scanCncnSmApp1Shot_ScrRelease, scanCncnSmApp1Shot_StartScan,
                     scanCncnSmApp1Shot_StopScan, scanCncnSm_NoOp, "One-shot application scan SM", SCAN_RESULT_TAG_APPLICATION_ONE_SHOT);

    /* Initialize the OS scan SM */
    scanCncnOsSm_Init ((TI_HANDLE)pScanCncn);

    /* initlaize the application scan result table */
    scanResultTable_Init (pScanCncn->hScanResultTable, pStadHandles, SCAN_RESULT_TABLE_DONT_CLEAR);
}

/** 
 * \fn     scanCncn_Recover
 * \brief  Recover the scan concentartor module
 * 
 * \param  hScanCncn - module handle
 * \return None
 */ 
void scanCncn_Recover (TI_HANDLE hScanCncn)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;
    uint8       uIndex = 0;

    /* stop scan guard timers for all clients */
    for (uIndex = 0; uIndex < SCAN_SCC_NUM_OF_CLIENTS; uIndex++) 
    {
        if (pScanCncn->pScanClients[uIndex]->hScanClientGuardTimer) 
        {
            tmr_StopTimer(pScanCncn->pScanClients[uIndex]->hScanClientGuardTimer);
        }
    }
}

/** 
 * \fn     scanCncn_SetDefaults 
 * \brief  Set registry values to scan concentrator
 * 
 * Set registry values to scan concentrator
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  pScanConcentratorInitParams - pointer to the registry parameters struct
 * \return None
 * \sa     scanCncn_Create, scanCncn_Init
 */ 
TI_STATUS scanCncn_SetDefaults (TI_HANDLE hScanCncn, TScanCncnInitParams *pScanCncnInitParams)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;
    uint8       uIndex     = 0;

    /* copy registry values */
    os_memoryCopy (pScanCncn->hOS, 
                   &pScanCncn->tInitParams, 
                   pScanCncnInitParams,
                   sizeof (TScanCncnInitParams));

    /* create scan guard timers for all clients */
    for (uIndex = 0; uIndex < SCAN_SCC_NUM_OF_CLIENTS; uIndex++) 
    {
        pScanCncn->pScanClients[uIndex]->hScanClientGuardTimer = tmr_CreateTimer(pScanCncn->hTimer);
        if ( pScanCncn->pScanClients[uIndex]->hScanClientGuardTimer == NULL)
        {
    		TRACE1(pScanCncn->hReport, REPORT_SEVERITY_INIT, "scanCncn_SetDefaults - "
                                                             "ERROR creating timer for client %d - "
                                                             "ABROTING init!\n", uIndex);
    		return TI_NOK;
        }
    }

    /* register SCR callbacks */
    scr_registerClientCB (pScanCncn->hSCR, SCR_CID_APP_SCAN, scanCncn_ScrAppCB, (TI_HANDLE)pScanCncn);
    scr_registerClientCB (pScanCncn->hSCR, SCR_CID_DRIVER_FG_SCAN, scanCncn_ScrDriverCB, (TI_HANDLE)pScanCncn);
    scr_registerClientCB (pScanCncn->hSCR, SCR_CID_CONT_SCAN, scanCncn_ScrRoamingContCB, (TI_HANDLE)pScanCncn);
    scr_registerClientCB (pScanCncn->hSCR, SCR_CID_IMMED_SCAN, scanCncn_ScrRoamingImmedCB, (TI_HANDLE)pScanCncn);

    /* register TWD scan complete CB */
	TWD_RegisterEvent (pScanCncn->hTWD,
					   TWD_OWN_EVENT_SCAN_CMPLT,
					   (void *)scanCncn_ScanCompleteCB,
					   (TI_HANDLE)pScanCncn); 

    TWD_RegisterEvent (pScanCncn->hTWD,
					   TWD_OWN_EVENT_SPS_SCAN_CMPLT, 
					   (void *)scanCncn_ScanCompleteCB, 
					   (TI_HANDLE)pScanCncn);


    /* register and enable periodic scan complete event with TWD */
    TWD_RegisterEvent (pScanCncn->hTWD, TWD_OWN_EVENT_PERIODIC_SCAN_COMPLETE,
                       (void *)scanCncn_PeriodicScanCompleteCB, (TI_HANDLE)pScanCncn);
    TWD_EnableEvent (pScanCncn->hTWD, TWD_OWN_EVENT_PERIODIC_SCAN_COMPLETE);

    /* and periodic scan report */
    TWD_RegisterEvent (pScanCncn->hTWD, TWD_OWN_EVENT_PERIODIC_SCAN_REPORT,
                       (void *)scanCncn_PeriodicScanReportCB, (TI_HANDLE)pScanCncn);
    TWD_EnableEvent (pScanCncn->hTWD, TWD_OWN_EVENT_PERIODIC_SCAN_REPORT);

    /* "register" the application scan result callback */
    scanCncn_RegisterScanResultCB ((TI_HANDLE)pScanCncn, SCAN_SCC_APP_ONE_SHOT, scanCncnApp_ApplicationScanResultCB, (TI_HANDLE)pScanCncn);
    scanCncn_RegisterScanResultCB ((TI_HANDLE)pScanCncn, SCAN_SCC_APP_PERIODIC, scanCncnApp_PeriodicScanResultCB, (TI_HANDLE)pScanCncn);

    /* set the Scan Result Aging threshold for the scan concentrator's Scan Result Table */
    scanResultTable_SetSraThreshold(pScanCncn->hScanResultTable, pScanCncnInitParams->uSraThreshold);

    /* set to the sme the handler of the scan concentrator Scan Result Table */
    sme_SetScanResultTable(pScanCncn->hSme, pScanCncn->hScanResultTable);

	return TI_OK;
}

/** 
 * \fn     scanCncn_TimerExpired
 * \brief  CB function to handle Scan guard timer expiration.
 * 
 * If Max time of consecutive expirations - trigger recovery.
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  bTwdInitOccured - Indicates if TWDriver recovery
 * occured since timer started \return None
 */ 
void scanCncn_TimerExpired (TI_HANDLE hScanCncnClient, TI_BOOL bTwdInitOccured)
{
	TScanCncnClient *pScanCncnClient = (TScanCncnClient*)hScanCncnClient;
	TScanCncn       *pScanCncn       = (TScanCncn*)pScanCncnClient->hScanCncn;

    
	pScanCncn->numOfConsTimerExpiry++;
	pScanCncn->bScanCompleteFlag = TI_FALSE;
    
	/* 
     No scan complete event will trigger recovery only after a consecutive configurable number of 
	 no scan complete events occurred.
     */
    if (pScanCncn->numOfConsTimerExpiry >= pScanCncn->tInitParams.numberOfNoScanCompleteToRecovery)
    {
        TRACE0( pScanCncn->hReport, REPORT_SEVERITY_ERROR, "Scan Timer expired. Starting recovery process.\n");

        pScanCncn->numOfConsTimerExpiry = 0;

        /* call the recovery module */
		healthMonitor_sendFailureEvent(pScanCncn->hHealthMonitor, NO_SCAN_COMPLETE_FAILURE);
    }
    else
    {
        TRACE2( pScanCncn->hReport, REPORT_SEVERITY_ERROR, "Scan Timer expired. consecutive failures: %d, threshold:%d, still not calling recovery.\n", pScanCncn->numOfConsTimerExpiry, pScanCncn->tInitParams.numberOfNoScanCompleteToRecovery);

        /* send Stop scan command */
		TWD_StopScan (pScanCncn->hTWD, pScanCncnClient->eScanTag, pScanCncn->eLatestScanType, NULL, NULL);
                      
        /* Call the Scan complete CB to handle unsuccessful completion */
        scanCncn_ScanCompleteNotification (pScanCncn,
										   pScanCncnClient->eScanTag,
										   0,
										   0xffff,
										   TI_FALSE, 
										   TI_OK);
	}
}

/** 
 * \fn     scanCncn_StopScanTimeoutExpired
 * \brief  CB function to handle Stop Scan guard timer expiration.
 *
 * Fakes a Scan Complete event
 *
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  bTwdInitOccured - Indicates if TWDriver recovery occured since timer started
 * \return none
 */
void scanCncn_StopScanTimeoutExpired (TI_HANDLE hScanCncnClient, TI_BOOL bTwdInitOccured)
{
	TScanCncnClient *pScanCncnClient = (TScanCncnClient*)hScanCncnClient;
	TScanCncn       *pScanCncn       = (TScanCncn*)pScanCncnClient->hScanCncn;

	pScanCncn->bScanCompleteFlag = TI_FALSE;

	TRACE0(pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_StopScanTimeoutExpired: Stop Scan timer expired\n");

	/* Call the Scan complete CB to handle unsuccessful completion */
	scanCncn_ScanCompleteNotification (pScanCncn,
			pScanCncnClient->eScanTag,
			0,
			0xffff,
			TI_FALSE,
			TI_OK);
}

/**
 * \fn     scanCncn_SwitchToConnected
 * \brief  Notifies the scan concentratoe the STA has connected to an infrastructure BSS
 * 
 * Notifies the scan concentratoe the STA has connected to an infrastructure BSS
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \return None
 * \sa     scanCncn_SwitchToNotConnected, scanCncn_SwitchToIBSS 
 */ 
void scanCncn_SwitchToConnected (TI_HANDLE hScanCncn)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE0(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_SwitchToConnected: Switching to connected state.\n");
    
    /* change connection status to connected */
    pScanCncn->eConnectionStatus = STA_CONNECTED;

    /* Any running scans in other modes will be aborted (if needed) by the SCR (or have already been) */
}

/** 
 * \fn     scanCncn_SwitchToNotConnected
 * \brief  Notifies the scan concentratoe the STA has disconnected from a BSS
 * 
 * Notifies the scan concentratoe the STA has disconnected from a BSS
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \return None
 * \sa     scanCncn_SwitchToConnected, scanCncn_SwitchToIBSS
 */ 
void scanCncn_SwitchToNotConnected (TI_HANDLE hScanCncn)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE0(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_SwitchToNotConnected: Switching to not connected state.\n");

    /* change connection status to connected */
    pScanCncn->eConnectionStatus = STA_NOT_CONNECTED;

    /* Any running scans in other modes will be aborted (if needed) by the SCR (or have already been) */
}

/** 
 * \fn     scanCncn_SwitchToIBSS
 * \brief  Notifies the scan concentratoe the STA has connected to an independent BSS
 * 
 * Notifies the scan concentratoe the STA has connected to an independent BSS
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \return None
 * \sa     scanCncn_SwitchToConnected, scanCncn_SwitchToNotConnected
 */ 
void scanCncn_SwitchToIBSS (TI_HANDLE hScanCncn)
{
    TScanCncn   *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE0(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_SwitchToIBSS: Switching to IBSS state.\n");

    /* change connection status to connected */
    pScanCncn->eConnectionStatus = STA_IBSS;

    /* Any running scans in other modes will be aborted (if needed) by the SCR (or have already been) */
}

EScanCncnResultStatus scanCncn_Start1ShotScan (TI_HANDLE hScanCncn, 
                                          EScanCncnClient eClient,
                                          TScanParams* pScanParams)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    paramInfo_t         *pParam;
    TI_STATUS           status = TI_OK;

    TRACE1(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_Start1ShotScan: Received scan request from client %d\n", eClient);

    /* assert eClient is valid */
    if (eClient >= SCAN_SCC_NUM_OF_CLIENTS) {
    	TRACE1(pScanCncn->hReport, REPORT_SEVERITY_WARNING, "scanCncn_Start1ShotScan: invalid eClient (%d). returning\n", eClient);
    	return SCAN_CRS_SCAN_FAILED;
    }

    pParam = (paramInfo_t *)os_memoryAlloc(pScanCncn->hOS, sizeof(paramInfo_t));
    if (!pParam) {
        return SCAN_CRS_SCAN_FAILED;
    }

    /* copy scan parameters to local buffer */
    os_memoryCopy (pScanCncn->hOS, &(pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams), 
                   pScanParams, sizeof(TScanParams));

    /* 
     * roaming scans (continuous and immediate) require to set the current SSID, to keep the scan manager
     * unaware of it, unless the SSID is broadcast (to allow customers to request broadcast scan)
     */
    if ((SCAN_SCC_ROAMING_CONT == eClient) || (SCAN_SCC_ROAMING_IMMED == eClient))
    {
        if (0 != pScanParams->desiredSsid.len)
        {
            /* set the SSID of the current AP */
            pParam->paramType = SME_DESIRED_SSID_ACT_PARAM;
            sme_GetParam (pScanCncn->hSme, pParam);
            os_memoryCopy (pScanCncn->hOS, 
                           &(pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.desiredSsid),
                           &(pParam->content.smeDesiredSSID),
                           sizeof(TSsid));
        }
    }
    os_memoryFree(pScanCncn->hOS, pParam, sizeof(paramInfo_t));

    /* ask the reg domain which channels are allowed for the requested scan type */
    status = scanCncn_VerifyChannelsWithRegDomain (hScanCncn, &(pScanCncn->pScanClients[ eClient ]->uScanParams), TI_FALSE);

    /* if no channels are available for scan, return negative result */
    if (TI_NOK == status ||
        0 == pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.numOfChannels)
    {
        TRACE0(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_Start1ShotScan: no cahnnels to scan after reg. domain verification, can't scan\n");
        return SCAN_CRS_SCAN_FAILED;
    }

    /* Mix the channel order in the 1 Shot Scan channel array */
    scanCncn_Mix1ShotScanChannels (pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.channelEntry, 
                                   pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.numOfChannels);

    if (TI_TRUE == pScanCncn->bUseSGParams)
    {
        scanCncn_SGupdateScanParams (hScanCncn, &(pScanCncn->pScanClients[ eClient ]->uScanParams), TI_FALSE);
    }
    
    /* mark that a scan request is in progress (to avoid client re-entrance if the scan fail) */
    pScanCncn->pScanClients[ eClient ]->bInRequest = TI_TRUE;

    /* mark the scan result as TI_OK (until other status will replace it) */
    pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_COMPLETE_OK;

    /* send a start scan event to the SM */
    genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_START, 
                 (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);

    /* mark that the scan request is no longer in progress */
    pScanCncn->pScanClients[ eClient ]->bInRequest = TI_FALSE;

    /* 
     * return scan result - if an error occured return the error, otherwise return running
     * (complete_ok is set to be returned when scan is complete)
     */
    if (SCAN_CRS_SCAN_COMPLETE_OK != pScanCncn->pScanClients[ eClient ]->eScanResult)
    {
        return pScanCncn->pScanClients[ eClient ]->eScanResult;
    }
    return SCAN_CRS_SCAN_RUNNING;
}

void scanCncn_StopScan (TI_HANDLE hScanCncn, EScanCncnClient eClient)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE1( pScanCncn->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_StopScan: Received stop scan request from client %d\n", eClient);

    /* if no previous error has occurred, change the state to stopped */
    if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ eClient ]->eScanResult)
    {
        pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_STOPPED;
    }

    /* send a stop scan event to the SM */
    genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_STOP, 
                 (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
}

/** 
 * \fn     scanCncn_StartPeriodicScan
 * \brief  Starts a periodic scan operation
 * 
 * Starts a periodic scan operation:
 *  - copies scan params to scan concentrator object
 *  - verifies the requested channels with the reg doamin
 *  - if needed, adjust to SG compensation values
 *  - send an event to the client SM
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eClient - the client requesting the scan operation
 * \param  pScanParams - scan parameters
 * \return Scan opeartion status
 * \retval SCAN_CRS_SCAN_RUNNING - scan started successfully and is now running
 * \retval SCAN_CRS_SCAN_FAILED - scan failed to start due to an unexpected error
 * \sa     scanCncn_StopPeriodicScan
 */
EScanCncnResultStatus scanCncn_StartPeriodicScan (TI_HANDLE hScanCncn,
                                                  EScanCncnClient eClient,
                                                  TPeriodicScanParams *pScanParams)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    TI_STATUS           status = TI_OK;

    TRACE1(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_startPeriodicScan: Received scan request from client %d\n", eClient);

    /* copy scan parameters to local buffer */
    os_memoryCopy (pScanCncn->hOS, &(pScanCncn->pScanClients[ eClient ]->uScanParams.tPeriodicScanParams), 
                   pScanParams, sizeof(TPeriodicScanParams));


    /* ask the reg domain which channels are allowed for the requested scan type */
/*    status = scanCncn_VerifyChannelsWithRegDomain (hScanCncn, &(pScanCncn->pScanClients[ eClient ]->uScanParams), TI_TRUE);*/
    status = scanCncn_PeriodicVerifyChannelsWithRegDomain (hScanCncn, &(pScanCncn->pScanClients[ eClient ]->uScanParams.tPeriodicScanParams));


    /* if no channels are available for scan, return negative result */
    if (TI_NOK == status ||
        0 == pScanCncn->pScanClients[ eClient ]->uScanParams.tPeriodicScanParams.uChannelNum)
    {
        TRACE0(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_StartPeriodicScan: no cahnnels to scan after reg. domain verification, can't scan\n");
        return SCAN_CRS_SCAN_FAILED;
    }

    /* Mix the channel order in the Periodic Scan channel array */
    scanCncn_MixPeriodicScanChannels (pScanCncn->pScanClients[ eClient ]->uScanParams.tPeriodicScanParams.tChannels, 
                                      pScanCncn->pScanClients[ eClient ]->uScanParams.tPeriodicScanParams.uChannelNum);

    if (TI_TRUE == pScanCncn->bUseSGParams)
    {
        scanCncn_SGupdateScanParams (hScanCncn, &(pScanCncn->pScanClients[ eClient ]->uScanParams), TI_TRUE);
    }
    
    /* mark that a scan request is in progress (to avoid client re-entrance if the scan fail) */
    pScanCncn->pScanClients[ eClient ]->bInRequest = TI_TRUE;

    /* mark the scan result as TI_OK (until other status will replace it) */
    pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_COMPLETE_OK;

    /* send a start scan event to the SM */
    genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_START, 
                 (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);

    /* mark that the scan request is no longer in progress */
    pScanCncn->pScanClients[ eClient ]->bInRequest = TI_FALSE;

    /* 
     * return scan result - if an error occured return the error, otherwise return running
     * (complete_ok is set to be returned when scan is complete)
     */
    if (SCAN_CRS_SCAN_COMPLETE_OK != pScanCncn->pScanClients[ eClient ]->eScanResult)
    {
        return pScanCncn->pScanClients[ eClient ]->eScanResult;
    }
    return SCAN_CRS_SCAN_RUNNING;
}

/** 
 * \fn     scanCncn_StopPeriodicScan 
 * \brief  Stop an on-going periodic scan operation
 * 
 * Set necessary flags and send a stop scan event to the client SM 
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eClient - the client requesting to stop the scan operation
 * \return None 
 * \sa     scanCncn_StartPeriodicScan
 */ 
void scanCncn_StopPeriodicScan (TI_HANDLE hScanCncn, EScanCncnClient eClient)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE1( pScanCncn->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_StopPeriodicScan: Received stop scan request from client %d\n", eClient);

    /* if no previous error has occurred, change the state to stopped */
    if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ eClient ]->eScanResult)
    {
        pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_STOPPED;
    }

    /* send a stop scan event to the SM */
    genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_STOP, 
                 (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
}

void scanCncn_RegisterScanResultCB (TI_HANDLE hScanCncn, EScanCncnClient eClient,
                                    TScanResultCB scanResultCBFunc, TI_HANDLE scanResultCBObj)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    /* save the function and object pointers */
    pScanCncn->pScanClients[ eClient ]->tScanResultCB = scanResultCBFunc;
    pScanCncn->pScanClients[ eClient ]->hScanResultCBObj = scanResultCBObj;
}

/**
 * \fn     scanCncn_ScanCompleteNotification
 * \brief  Called when a scan is complete
 * 
 * Update scan status and send a complete event to the SM
 * 
 * \note   Must wait for all results to be received before the scan is actually completed
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eTag - the scan type tag
 * \param  uResoultCount - number of results received during this scan
 * \param  SPSStatus - which channels were attempted (if this is an SPS scan)
 * \param  bTSFError - whether a TSF error occurred (if this is an SPS scan)
 * \param  scanStatus - scan SRV status (OK / NOK)
 * \return None
 */ 
void scanCncn_ScanCompleteNotification (TI_HANDLE hScanCncn, EScanResultTag eTag,
										TI_UINT32 uResultCount, TI_UINT16 SPSStatus,
										TI_BOOL bTSFError, TI_STATUS scanStatus)
										
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    EScanCncnClient     eClient;

    if (hScanCncn == NULL )
    {
        return;
    }

    TRACE5(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScanCompleteNotificationCB: tag: %d, result count: %d, SPS status: %d, TSF Error: %d, scan status: %d\n", eTag, uResultCount, SPSStatus, bTSFError, scanStatus);

    /* get the scan client value from the scan tag */
    eClient = SCAN_CLIENT_FROM_TAG (eTag);

    if (eClient >= SCAN_SCC_NUM_OF_CLIENTS) /* Sanity check should not happen.
                                               if it happens the scan timer will expire and scan will eventualy fail */
    {
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_ScanCompleteNotificationCB: eClient = %d >= SCAN_SCC_NUM_OF_CLIENTS\n", eClient);
        return;
    }

    /* update scan result if scan SRV reported error (and no error occured so far) */
    if ((TI_OK != scanStatus) && (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ eClient ]->eScanResult))
    {
        pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_FAILED;
    }

    /* for SPS: copy SPS result and update scan status according to TSF error */
    if (SCAN_TYPE_SPS == pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.scanType)
    {
        pScanCncn->pScanClients[ eClient ]->uSPSScanResult = SPSStatus;
        if (TI_TRUE == bTSFError)
        {
            pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_TSF_ERROR;
        }
    }

    /* update number of frames expected */
    pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber += uResultCount;

    /* check if all frames had been received */
    if (pScanCncn->pScanClients[ eClient ]->uResultCounter >= pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber)
        
    {
        TRACE2(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScanCompleteNotificationCB: client %d received %d results, matching scan complete FW indication, sending scan complete event\n", eClient, pScanCncn->pScanClients[ eClient ]->uResultCounter);

        /* all frames had been received, send a scan complete event to the client SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_SCAN_COMPLETE, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
    }
    else
    {
        TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScanCompleteNotificationCB: client %d received %d results, FW indicated %d results, waiting for more\n", eClient, pScanCncn->pScanClients[ eClient ]->uResultCounter, pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber);

        /* still waiting for some frames, turn on the scan complete pending flag */
        pScanCncn->pScanClients[ eClient ]->bScanCompletePending = TI_TRUE;
    }
}

/** 
 * \fn     scanCncn_PeriodicScanReportCB
 * \brief  Called when results are available but periodic scan is not yet complete
 * 
 * Called when results are available but periodic scan is not yet complete
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  str - event data
 * \param  strLen - data length
 * \return None
 * \sa     scanCncn_PeriodicScanCompleteCB
 */ 
void scanCncn_PeriodicScanReportCB (TI_HANDLE hScanCncn, char* str, TI_UINT32 strLen)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    EScanCncnClient     eClient;
    EScanResultTag      eTag = (EScanResultTag)str[ 1 ];
    TI_UINT32           uResultCount = str[ 0 ];

    TRACE2(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_PeriodicScanReportCB: tag: %d, result count: %d\n", eTag, uResultCount);

    /* get the scan client value from the scan tag */
    eClient = SCAN_CLIENT_FROM_TAG (eTag);

    /* update number of frames expected */
    pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber += uResultCount;
}
/** 
 * \fn     scanCncn_PeriodicScanReportCB
 * \brief  Called when results are available but the scan is bot yet complete
 * 
 * Called when results are available but the scan is bot yet complete
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  str - event data
 * \param  strLen - data length
 * \return None
 * \sa     scanCncn_PeriodicScanReportCB
 */
void scanCncn_PeriodicScanCompleteCB (TI_HANDLE hScanCncn, char* str, TI_UINT32 strLen)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    EScanCncnClient     eClient;
    EScanResultTag      eTag = (EScanResultTag)str[1];
    TI_UINT32           uResultCount = (TI_UINT8)(str[0]);

    TRACE2(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_PeriodicScanCompleteCB: tag: %d, result count: %d\n", eTag, uResultCount);

    /* get the scan client value from the scan tag */
    eClient = SCAN_CLIENT_FROM_TAG (eTag);

    /* update number of frames expected */
    pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber += uResultCount;

    /* check if all frames had been received */
    if (pScanCncn->pScanClients[ eClient ]->uResultCounter >= pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber)
    {
        TRACE2(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_PeriodicScanCompleteCB: client %d received %d results, matching scan complete FW indication, sending scan complete event\n", eClient, pScanCncn->pScanClients[ eClient ]->uResultCounter);
        /* all frames had been received, send a scan complete event to the client SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_SCAN_COMPLETE, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
    }
    else
    {
        TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_PeriodicScanCompleteCB: client %d received %d results, FW indicated %d results, waiting for more\n", eClient, pScanCncn->pScanClients[ eClient ]->uResultCounter, pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber);
        /* still waiting for some frames, turn on the scan complete pending flag */
        pScanCncn->pScanClients[ eClient ]->bScanCompletePending = TI_TRUE;
    }
}

/** 
 * \fn     scanCncn_MlmeResultCB
 * \brief  Handles an MLME result (received frame)
 * 
 * Filters the frame for roaming scans and passes it to the appropriate client
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  bssid - a pointer to the address of the AP sending this frame
 * \param  frameInfo - the IE in the frame
 * \param  pRxAttr - a pointer to TNET RX attributes struct
 * \param  buffer - a pointer to the frame body
 * \param  bufferLength - the frame body length
 * \return None
 */ 
void scanCncn_MlmeResultCB (TI_HANDLE hScanCncn, TMacAddr* bssid, mlmeFrameInfo_t* frameInfo, 
                            TRxAttr* pRxAttr, TI_UINT8* buffer, TI_UINT16 bufferLength)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    EScanCncnClient     eClient;
    TScanFrameInfo      tScanFrameInfo;
    TI_BOOL             bValidResult = TI_TRUE;

    /* get the scan client value from the scan tag */
    eClient = SCAN_CLIENT_FROM_TAG (pRxAttr->eScanTag);

    /* increase scan result counter */
    pScanCncn->pScanClients[ eClient ]->uResultCounter++;

    /* 
     * erroneous results are signaled by NULL pointers and are notified to the scan concentrator to 
     * update the counter only! 
     */
    if (NULL == bssid)
    {
        TRACE0(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: received an empty frame notification from MLME\n");

        /* invalid resuilt */
        bValidResult = TI_FALSE;
    }
    /* are results valid so far (TI_TRUE == bValidResult) */
    else
    {
        TRACE6(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: received frame from BBSID: %02x:%02x:%02x:%02x:%02x:%02x\n", (*bssid)[ 0 ], (*bssid)[ 1 ], (*bssid)[ 2 ], (*bssid)[ 3 ], (*bssid)[ 4 ], (*bssid)[ 5 ]);

        /* If SSID IE is missing, discard the frame */
        if (frameInfo->content.iePacket.pSsid == NULL)
        {
            TRACE6(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: discarding frame from BSSID: %02x:%02x:%02x:%02x:%02x:%02x, because SSID IE is missing!!\n", (*bssid)[ 0 ], (*bssid)[ 1 ], (*bssid)[ 2 ], (*bssid)[ 3 ], (*bssid)[ 4 ], (*bssid)[ 5 ]);
            bValidResult = TI_FALSE;
        }

        /* If SSID length is 0 (hidden SSID) */
        else if (frameInfo->content.iePacket.pSsid->hdr[1] == 0)
        {
			/* Discard the frame unless it is application scan for any SSID - In this case we want to see also the hidden SSIDs*/
            if  (!(((SCAN_SCC_APP_ONE_SHOT == eClient) || (SCAN_SCC_APP_PERIODIC == eClient)) &&
				    pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.desiredSsid.len == 0))
			{
				TRACE6(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: discarding frame from BSSID: %02x:%02x:%02x:%02x:%02x:%02x, because SSID is hidden (len=0)\n", (*bssid)[ 0 ], (*bssid)[ 1 ], (*bssid)[ 2 ], (*bssid)[ 3 ], (*bssid)[ 4 ], (*bssid)[ 5 ]);
				bValidResult = TI_FALSE;
			}
        }

       /* 
        * for roaming continuous and immediate, discard frames from current AP, 
        * or frames with SSID different than desired when the scan is NOT SPS
        */
        else if ((SCAN_SCC_ROAMING_CONT == eClient) || (SCAN_SCC_ROAMING_IMMED == eClient))
        {
            bssEntry_t *pCurrentAP;

            pCurrentAP = apConn_getBSSParams(pScanCncn->hAPConn);
            if (NULL == pCurrentAP) /* sanity check, this should not happen since continues and immediate scan are
                                       performed only when connected therfore currentAp should not be NULL */
            {
                TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_MlmeResultCB: client = %d, pCurrentAP is NULL\n", eClient);
                return;
            }

            if(MAC_EQUAL(*bssid, pCurrentAP->BSSID) ||
               ((os_memoryCompare (pScanCncn->hOS,
                                   (TI_UINT8*)frameInfo->content.iePacket.pSsid->serviceSetId, 
                                   (TI_UINT8*)pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.desiredSsid.str, 
                                   pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.desiredSsid.len)) &&
                pScanCncn->pScanClients[ eClient ]->uScanParams.tOneShotScanParams.scanType != SCAN_TYPE_SPS))
            {
                TRACE6(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: discarding frame from SSID: , BSSID: %02x:%02x:%02x:%02x:%02x:%02x, because SSID different from desired or from current AP!\n", (*bssid)[ 0 ], (*bssid)[ 1 ], (*bssid)[ 2 ], (*bssid)[ 3 ], (*bssid)[ 4 ], (*bssid)[ 5 ]);
                bValidResult = TI_FALSE;
            }
        }

        /* if rssi is lower than the Rssi threshold, discard frame */
        if ( pRxAttr->Rssi < pScanCncn->tInitParams.nRssiThreshold )
        {
            TRACE7(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: discarding frame from BSSID: %02x:%02x:%02x:%02x:%02x:%02x, because RSSI = %d\n", (*bssid)[ 0 ], (*bssid)[ 1 ], (*bssid)[ 2 ], (*bssid)[ 3 ], (*bssid)[ 4 ], (*bssid)[ 5 ], pRxAttr->Rssi);
            bValidResult = TI_FALSE;
        }

        if(TI_TRUE == bValidResult)
        {
            /* build the scan frame info object */
            tScanFrameInfo.bssId = bssid;
            tScanFrameInfo.band = (ERadioBand)pRxAttr->band;
            tScanFrameInfo.channel = pRxAttr->channel;
            tScanFrameInfo.parsedIEs = frameInfo;
            tScanFrameInfo.rate = pRxAttr->Rate;
            tScanFrameInfo.rssi = pRxAttr->Rssi;
            tScanFrameInfo.snr = pRxAttr->SNR;
            tScanFrameInfo.staTSF = pRxAttr->TimeStamp;
            tScanFrameInfo.buffer = buffer;
            tScanFrameInfo.bufferLength = bufferLength;

            if (TI_TRUE == pScanCncn->tInitParams.bPushMode)
            {
               /* 
                * The scan found result, send a scan report event to the use with the frame 	parameters without save the result in the scan table 
                */
                EvHandlerSendEvent (pScanCncn->hEvHandler, IPC_EVENT_SCAN_REPORT, (TI_UINT8*)&tScanFrameInfo, sizeof(TScanFrameInfo));
            }
            else
            {
               /* call the client result CB */
               pScanCncn->pScanClients[ eClient ]->tScanResultCB (pScanCncn->pScanClients[ eClient ]->hScanResultCBObj,
                                                                  SCAN_CRS_RECEIVED_FRAME, &tScanFrameInfo, 0xffff ); /* SPS status is only valid on SPS scan complete */
            }
        }
    }

    /* check if scan complete is pending for this frame for all results */
    if((TI_TRUE == pScanCncn->pScanClients[ eClient ]->bScanCompletePending) &&
       (pScanCncn->pScanClients[ eClient ]->uResultCounter == pScanCncn->pScanClients[ eClient ]->uResultExpectedNumber))
    {
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_MlmeResultCB: received frame number %d, scan complete pending, sending scan complet event\n", pScanCncn->pScanClients[ eClient ]->uResultCounter);

        /* send a scan complete event to the client SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_SCAN_COMPLETE, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
    }
}

/** 
 * \fn     scanCncn_ScrRoamingImmedCB
 * \brief  Called by SCR for immediate roaming client status change notification
 * 
 * Handles status change by sending the appropriate SM event
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eRrequestStatus - the immediate scan for roaming client status
 * \param  eResource - the resource for which the CB is issued
 * \param  ePendreason - The reason for pend status, if the status is pend
 * \return None
 */ 
void scanCncn_ScrRoamingImmedCB (TI_HANDLE hScanCncn, EScrClientRequestStatus eRequestStatus,
                                 EScrResourceId eResource, EScePendReason ePendReason)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScrRoamingImmedCB: status: %d, resource: %d pend reason: %d\n", eRequestStatus, eResource, ePendReason);

    /* act according to the request staus */
    switch (eRequestStatus)
    {
    case SCR_CRS_RUN:
        /* send an SCR run event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->hGenSM, SCAN_CNCN_SM_EVENT_RUN, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]);
        break;

    case SCR_CRS_PEND:
        /* if pending reason has changed to different group - send a reject event 
           (should only happen when pending) */
        if ( SCR_PR_DIFFERENT_GROUP_RUNNING == ePendReason )
        {
            /* send an SCR reject event to the SM - would not scan when not performing roaming */
            pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->eScanResult = SCAN_CRS_SCAN_FAILED;
            genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->hGenSM, SCAN_CNCN_SM_EVENT_REJECT, 
                         (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]);
        }
        break;

    case SCR_CRS_FW_RESET:
        /* if no previous error has occurred, change the state to FW reset */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->eScanResult)
        {
            pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->eScanResult = SCAN_CRS_SCAN_ABORTED_FW_RESET;
        }

        /* send a recovery event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]->hGenSM, SCAN_CNCN_SM_EVENT_RECOVERY, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_IMMED ]);
        break;

    case SCR_CRS_ABORT:
        /* This should never happen, report error */
    default:
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_ScrRoamingImmedCB: Illegal SCR request status: %d.\n", eRequestStatus);
        break;
    }
}

/** 
 * \fn     scanCncn_ScrRoamingContCB
 * \brief  Called by SCR for continuous roaming client status change notification
 * 
 * Handles status change by sending the appropriate SM event
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eRrequestStatus - the continuous scan for roaming client status
 * \param  eResource - the resource for which the CB is issued
 * \param  ePendreason - The reason for pend status, if the status is pend
 * \return None
 */ 
void scanCncn_ScrRoamingContCB (TI_HANDLE hScanCncn, EScrClientRequestStatus eRequestStatus,
                                EScrResourceId eResource, EScePendReason ePendReason)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScrRoamingContCB: status: %d, resource: %d pend reason: %d\n", eRequestStatus, eResource, ePendReason);

    /* act according to the request staus */
    switch (eRequestStatus)
    {
    case SCR_CRS_RUN:
        /* send an SCR run event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->hGenSM, SCAN_CNCN_SM_EVENT_RUN, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]);
        break;

    case SCR_CRS_PEND:
        /* if pending reason has changed to different group - send a reject event (should only happen when pending) */
        if ( SCR_PR_DIFFERENT_GROUP_RUNNING == ePendReason )
        {
            pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->eScanResult = SCAN_CRS_SCAN_FAILED;
            genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->hGenSM, SCAN_CNCN_SM_EVENT_REJECT, 
                         (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]);
        }
        break;

    case SCR_CRS_FW_RESET:
        /* if no previous error has occurred, change the state to FW reset */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->eScanResult)
        {
            pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->eScanResult = SCAN_CRS_SCAN_ABORTED_FW_RESET;
        }

        /* send a recovery event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->hGenSM, SCAN_CNCN_SM_EVENT_RECOVERY, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]);
        break;


    case SCR_CRS_ABORT:
        
        /* if no previous error has occurred, change the result to abort */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->eScanResult)
        {
            pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->eScanResult = SCAN_CRS_SCAN_ABORTED_HIGHER_PRIORITY;
        }

        /* send an abort scan event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]->hGenSM, SCAN_CNCN_SM_EVENT_ABORT, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_ROAMING_CONT ]);
        break;

    default:
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_ScrRoamingContCB: Illegal SCR request status: %d.\n", eRequestStatus);
        break;
    }
}

/** 
 * \fn     scanCncn_ScrAppCB
 * \brief  Called by SCR for application scan client status change notification
 * 
 * Handles status change by sending the appropriate SM event
 * 
 * \note   this function is used by the SCR for both one-shot and periodic application scan
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eRrequestStatus - the application scan client status
 * \param  eResource - the resource for which the CB is issued
 * \param  ePendreason - The reason for pend status, if the status is pend
 * \return None
 */ 
void scanCncn_ScrAppCB (TI_HANDLE hScanCncn, EScrClientRequestStatus eRequestStatus,
                        EScrResourceId eResource, EScePendReason ePendReason )
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    EScanCncnClient     eClient;

    TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScrAppCB: status: %d, resource: %d pend reason: %d\n", eRequestStatus, eResource, ePendReason);

    /* set client according to SCr resource */
    if (SCR_RESOURCE_PERIODIC_SCAN == eResource)
    {
        eClient = SCAN_SCC_APP_PERIODIC;
    }
    else
    {
        eClient = SCAN_SCC_APP_ONE_SHOT;
    }

    /* act according to the request staus */
    switch (eRequestStatus)
    {
    /* 
     * Note: pend is not handled because application scan cancel its scan request when it receives pend
     * as the SCR request result, and thus it is assumed that the application scan request will never be
     * pending
     */
    case SCR_CRS_RUN:
        /* send an SCR run event to the SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_RUN, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
        break;

    case SCR_CRS_FW_RESET:
        /* if no previous error has occurred, change the state to FW reset */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ eClient ]->eScanResult)
        {
            pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_ABORTED_FW_RESET;
        }

        /* send a recovery event to the SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_RECOVERY, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
        break;

    case SCR_CRS_ABORT:  
        
        /* if no previous error has occurred, change the result to abort */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ eClient ]->eScanResult)
        {
            pScanCncn->pScanClients[ eClient ]->eScanResult = SCAN_CRS_SCAN_ABORTED_HIGHER_PRIORITY;
        }

        /* send an abort scan event to the SM */
        genSM_Event (pScanCncn->pScanClients[ eClient ]->hGenSM, SCAN_CNCN_SM_EVENT_ABORT, 
                     (TI_HANDLE)pScanCncn->pScanClients[ eClient ]);
        break;

    default:
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_ScrAppCB: Illegal SCR request status: %d.\n", eRequestStatus);
        break;
    }
}

/** 
 * \fn     scanCncn_ScrDriverCB
 * \brief  Called by SCR for driver scan client status change notification
 * 
 * Handles status change by sending the appropriate SM event
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  eRrequestStatus - the driver scan client status
 * \param  eResource - the resource for which the CB is issued
 * \param  ePendreason - The reason for pend status, if the status is pend
 * \return None
 */ 
void scanCncn_ScrDriverCB (TI_HANDLE hScanCncn, EScrClientRequestStatus eRequestStatus,
                           EScrResourceId eResource, EScePendReason ePendReason)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    TRACE3(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_ScrDriverCB: status: %d, resource: %d pend reason: %d\n", eRequestStatus, eResource, ePendReason);

    /* act according to the request staus */
    switch (eRequestStatus)
    {
    case SCR_CRS_RUN:
        /* send the next event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->hGenSM, SCAN_CNCN_SM_EVENT_RUN, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]);
        break;

    case SCR_CRS_PEND:
        /* if pending reason has changed to different group - send a reject event (should only happen when pending) */
        if ( SCR_PR_DIFFERENT_GROUP_RUNNING == ePendReason )
        {
            pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->eScanResult = SCAN_CRS_SCAN_FAILED;
            genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->hGenSM, SCAN_CNCN_SM_EVENT_REJECT, 
                         (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]);
        }
        break;

    case SCR_CRS_FW_RESET:
        /* if no previous error has occurred, change the state to FW reset */
        if (SCAN_CRS_SCAN_COMPLETE_OK == pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->eScanResult)
        {
            pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->eScanResult = SCAN_CRS_SCAN_ABORTED_FW_RESET;
        }

        /* send a recovery event to the SM */
        genSM_Event (pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]->hGenSM, SCAN_CNCN_SM_EVENT_RECOVERY, 
                     (TI_HANDLE)pScanCncn->pScanClients[ SCAN_SCC_DRIVER ]);
        break;
    
    case SCR_CRS_ABORT:
    /* This should never happen, report error */
    default:
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR , "scanCncn_ScrDriverCB: Illegal SCR request status: %d.\n", eRequestStatus);
        break;
    }
}

/** 
 * \fn     scanCncn_VerifyChannelsWithRegDomain
 * \brief  Verifies channel validity and TX power with the reg. domain
 * 
 * Verifies channel validity and TX power with the reg. domain. Removes invalid channels.
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  puScanParams - a pointer to the scan parmeters union
 * \param  bPeriodicScan - TRUE if the parameters are for periodic scan, FALSE if for one-shot scan
 * \return TI_OK if scanParams is legal (channel list may contain 0 entries)
 * 		   TI_NOK if the scanParams were illegal (i.e. number of channels to big)
 */ 
TI_STATUS scanCncn_VerifyChannelsWithRegDomain (TI_HANDLE hScanCncn, UScanParams *puScanParams, TI_BOOL bPeriodicScan)
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    paramInfo_t         *pParam;
    paramInfo_t         tDfsParam;
    TI_UINT8            i, uChannelNum;

    TI_BOOL          bRegulatoryDomainEnabled;
    TI_BOOL          bForceBGPassive    = TI_FALSE;
    TI_BOOL          bForceAPassive 	= TI_FALSE;




    pParam = (paramInfo_t *)os_memoryAlloc(pScanCncn->hOS, sizeof(paramInfo_t));
    if (!pParam) {
        return TI_NOK;
    }

    /* query the regulatory domain if 802.11d is in use */
    pParam->paramType = REGULATORY_DOMAIN_ENABLED_PARAM;
    regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam );
    bRegulatoryDomainEnabled = pParam->content.regulatoryDomainEnabled;

    /* Get country code status for 5.2 */
    pParam->paramType          = REGULATORY_DOMAIN_IS_COUNTRY_FOUND;
    pParam->content.eRadioBand = RADIO_BAND_5_0_GHZ;
    regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam);

    /* scan type is passive if 802.11d is enabled and country IE was not yet found, active otherwise */
    if ((TI_TRUE == bRegulatoryDomainEnabled) && (TI_FALSE == pParam->content.bIsCountryFound))
    {
        bForceAPassive = TI_TRUE;
    }

    /* Get country code status for 2.4*/
    pParam->content.eRadioBand = RADIO_BAND_2_4_GHZ;
    regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam);    
    if ((TI_TRUE == bRegulatoryDomainEnabled) && (TI_FALSE == pParam->content.bIsCountryFound))
    {
        bForceBGPassive = TI_TRUE;
    }


    /* get channel number according to scan type */
    if (TI_TRUE == bPeriodicScan)
    {
        uChannelNum = puScanParams->tPeriodicScanParams.uChannelNum;
        if (uChannelNum > PERIODIC_SCAN_MAX_CHANNEL_NUM)
        {
            TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_VerifyChannelsWithRegDomain: uChannelNum = %d > PERIODIC_SCAN_MAX_CHANNEL_NUM\n", uChannelNum);
            return TI_NOK;
        }
    }
    else
    {   
        uChannelNum = puScanParams->tOneShotScanParams.numOfChannels; 
        if (uChannelNum > MAX_NUMBER_OF_CHANNELS_PER_SCAN) 
        {
            TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_VerifyChannelsWithRegDomain: uChannelNum = %d > MAX_NUMBER_OF_CHANNELS_PER_SCAN\n", uChannelNum);
            return TI_NOK;
        }
        if ( ((puScanParams->tOneShotScanParams.band == RADIO_BAND_5_0_GHZ) && (TI_TRUE == bForceAPassive)) ||
            ((puScanParams->tOneShotScanParams.band == RADIO_BAND_2_4_GHZ) && (TI_TRUE == bForceBGPassive)) ||
            ((puScanParams->tOneShotScanParams.band == RADIO_BAND_DUAL) && ((TI_TRUE == bForceBGPassive) || (TI_TRUE == bForceAPassive)) ) )
        {
            puScanParams->tOneShotScanParams.scanType = SCAN_TYPE_NORMAL_PASSIVE;
        }
    }

    /* check channels */
    for (i = 0; i < uChannelNum; )
    {  

        /* Note that i is only increased when channel is valid - if channel is invalid, another 
             channel is copied in its place, and thus the same index should be checked again. However,
             since the number of channels is decreased, the loop end condition is getting nearer! */

        if (TI_TRUE == bPeriodicScan)
        {
            EDot11Mode siteMgrDot11Mode;
            /* Check channel validity according to single/dual band*/
            pParam->paramType = SITE_MGR_DESIRED_DOT11_MODE_PARAM;
            siteMgr_getParam(pScanCncn->hSiteManager, pParam);

            siteMgrDot11Mode = pParam->content.siteMgrDot11Mode;

            /* set channel for periodic scan */
            pParam->content.channelCapabilityReq.channelNum =
                puScanParams->tPeriodicScanParams.tChannels[ i ].uChannel;

            if ((siteMgrDot11Mode != DOT11_A_MODE) &&
                (siteMgrDot11Mode != DOT11_DUAL_MODE) &&
                (pParam->content.channelCapabilityReq.channelNum > A_5G_BAND_MIN_CHANNEL))
    
            {
                pParam->content.channelCapabilityRet.channelValidity = TI_FALSE;
            }
            else
            {
                pParam->paramType = REGULATORY_DOMAIN_GET_SCAN_CAPABILITIES;

                /* set band and scan type for periodic scan */
                pParam->content.channelCapabilityReq.band = puScanParams->tPeriodicScanParams.tChannels[ i ].eBand;
    
                if ((puScanParams->tPeriodicScanParams.tChannels[ i ].eScanType == SCAN_TYPE_NORMAL_PASSIVE) ||
                ((pParam->content.channelCapabilityReq.band == RADIO_BAND_5_0_GHZ) && (TI_TRUE == bForceAPassive)) ||
                ((pParam->content.channelCapabilityReq.band == RADIO_BAND_2_4_GHZ) && (TI_TRUE == bForceBGPassive)) ||
                ((puScanParams->tOneShotScanParams.band == RADIO_BAND_DUAL) && ((TI_TRUE == bForceBGPassive) || (TI_TRUE == bForceAPassive)) ) )
                {
                    pParam->content.channelCapabilityReq.scanOption = PASSIVE_SCANNING;
                puScanParams->tPeriodicScanParams.tChannels[ i ].eScanType = SCAN_TYPE_NORMAL_PASSIVE;
                }
                else
                {
                    /* query the reg. domain whether this is a DFS channel */
                    tDfsParam.paramType = REGULATORY_DOMAIN_IS_DFS_CHANNEL;
                    tDfsParam.content.tDfsChannel.eBand = puScanParams->tPeriodicScanParams.tChannels[ i ].eBand;
                    tDfsParam.content.tDfsChannel.uChannel = puScanParams->tPeriodicScanParams.tChannels[ i ].uChannel;
                    regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, &tDfsParam);
    
                    /* if this is a DFS channel */
                    if (TI_TRUE == tDfsParam.content.tDfsChannel.bDfsChannel)
                    {
                        /* 
                         * DFS channels are first scanned passive and than active, so reg. domain can only validate
                         * these channels for pasiive scanning at the moment
                         */
                        pParam->content.channelCapabilityReq.scanOption = PASSIVE_SCANNING;
    
                        /* set the channel scan type to DFS */
					if (pScanCncn->tInitParams.bEnablePassivBActive)
					{
                        puScanParams->tPeriodicScanParams.tChannels[ i ].eScanType = SCAN_TYPE_PACTSIVE;
					}
					else
					{
						puScanParams->tPeriodicScanParams.tChannels[ i ].eScanType = SCAN_TYPE_NORMAL_PASSIVE;
					}
                        /* passive scan time is passed for all channels to the TWD, and copied there to the FW */
                    }
                    else
                    {
                        pParam->content.channelCapabilityReq.scanOption = ACTIVE_SCANNING;
                    }
                }
                regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam);

            }
            if (TI_FALSE == pParam->content.channelCapabilityRet.channelValidity)
            { 
                if (i < uChannelNum - 1)
                {
                    /* channel not allowed - copy the rest of the channel in its place */
                    os_memoryCopy (pScanCncn->hOS, &(puScanParams->tPeriodicScanParams.tChannels[ i ]),
                               &(puScanParams->tPeriodicScanParams.tChannels[ i + 1 ]),
                               sizeof(TPeriodicChannelEntry) * (puScanParams->tPeriodicScanParams.uChannelNum - i - 1));
                
                }
                puScanParams->tPeriodicScanParams.uChannelNum--;
                uChannelNum--;
            }
            else
            {
                /* also set the power level to the minimumm between requested power and allowed power */
                puScanParams->tPeriodicScanParams.tChannels[ i ].uTxPowerLevelDbm = 
                        TI_MIN( pParam->content.channelCapabilityRet.maxTxPowerDbm, 
                                puScanParams->tPeriodicScanParams.tChannels[ i ].uTxPowerLevelDbm ); 

                i += 1;
            }
        }
        else
        {

            pParam->paramType = REGULATORY_DOMAIN_GET_SCAN_CAPABILITIES;

            /* set band and scan type for one-shot scan */
            pParam->content.channelCapabilityReq.band = puScanParams->tOneShotScanParams.band;
            if ((puScanParams->tOneShotScanParams.scanType == SCAN_TYPE_NORMAL_PASSIVE) ||
                (puScanParams->tOneShotScanParams.scanType == SCAN_TYPE_TRIGGERED_PASSIVE) ||
                (puScanParams->tOneShotScanParams.scanType == SCAN_TYPE_SPS))
            {
                pParam->content.channelCapabilityReq.scanOption = PASSIVE_SCANNING;
            }
            else
            {
                pParam->content.channelCapabilityReq.scanOption = ACTIVE_SCANNING;
            }
        
            /* set channel for one-shot scan - SPS */
            if (SCAN_TYPE_SPS == puScanParams->tOneShotScanParams.scanType)
            {
                pParam->content.channelCapabilityReq.channelNum = 
                    puScanParams->tOneShotScanParams.channelEntry[ i ].SPSChannelEntry.channel;
                regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam);
                if (TI_FALSE == pParam->content.channelCapabilityRet.channelValidity)
                {   
                    if (i < uChannelNum - 1)
                    {
                        /* channel not allowed - copy the rest of the channel in its place */
                        os_memoryCopy (pScanCncn->hOS, &(puScanParams->tOneShotScanParams.channelEntry[ i ]),
                                   &(puScanParams->tOneShotScanParams.channelEntry[ i + 1 ]), 
                                   sizeof(TScanChannelEntry) * (puScanParams->tOneShotScanParams.numOfChannels - i - 1));
                    
                    }
                    puScanParams->tOneShotScanParams.numOfChannels--;
                    uChannelNum--;
                }
                else
                {
                    i += 1;
                }
                
            }
            /* set channel for one-shot scan - all other scan types */
            else
            {
                pParam->content.channelCapabilityReq.channelNum = 
                    puScanParams->tOneShotScanParams.channelEntry[ i ].normalChannelEntry.channel;
                regulatoryDomain_getParam (pScanCncn->hRegulatoryDomain, pParam);
                if (TI_FALSE == pParam->content.channelCapabilityRet.channelValidity)
                {   
                    if (i < uChannelNum - 1)
                    {
                        /* channel not allowed - copy the rest of the channel in its place */
                        os_memoryCopy (pScanCncn->hOS, &(puScanParams->tOneShotScanParams.channelEntry[ i ]),
                                  &(puScanParams->tOneShotScanParams.channelEntry[ i + 1 ]), 
                                  sizeof(TScanChannelEntry) * (puScanParams->tOneShotScanParams.numOfChannels - i - 1));
                    }
                    puScanParams->tOneShotScanParams.numOfChannels--;
                    uChannelNum--;
                }
                else
                {

                    puScanParams->tOneShotScanParams.channelEntry[i].normalChannelEntry.txPowerDbm = 
                            TI_MIN (pParam->content.channelCapabilityRet.maxTxPowerDbm, 
                                    puScanParams->tOneShotScanParams.channelEntry[i].normalChannelEntry.txPowerDbm);
                    i += 1;
                }
            }
        }
    }    
    os_memoryFree(pScanCncn->hOS, pParam, sizeof(paramInfo_t));
    return TI_OK;
}

TI_STATUS scanCncn_PeriodicVerifyChannelsWithRegDomain (TI_HANDLE hScanCncn, TPeriodicScanParams *pPeriodicScanParams)
{
    TScanCncn* pScanCncn = (TScanCncn*)hScanCncn;
    TI_UINT32 uChannelNum = pPeriodicScanParams->uChannelNum;
    TI_UINT32 i;

    paramInfo_t* pParam = (paramInfo_t *)os_memoryAlloc(pScanCncn->hOS, sizeof(paramInfo_t));

    if (!pParam) {
        return TI_NOK;
    }

    if (uChannelNum > PERIODIC_SCAN_MAX_CHANNEL_NUM)
    {
        os_memoryFree(pScanCncn->hOS, pParam, sizeof(paramInfo_t));
        TRACE1(pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_PeriodicVerifyChannelsWithRegDomain: uChannelNum = %d > PERIODIC_SCAN_MAX_CHANNEL_NUM\n", uChannelNum);
        return TI_NOK;
    }

    for (i = 0; i < uChannelNum;)
    {
        TI_UINT32 channel = pPeriodicScanParams->tChannels[i].uChannel;
        TI_STATUS status;

        pParam->paramType = REGULATORY_DOMAIN_GET_CHANNEL_CAPABILITY;
        pParam->content.channelValidity.band = pPeriodicScanParams->tChannels[i].eBand;
        pParam->content.channelValidity.channelNum = pPeriodicScanParams->tChannels[i].uChannel;
        status = regulatoryDomain_getParam(pScanCncn->hRegulatoryDomain, pParam);

        /* If regulatoryDomain_getParam return an error status OR
         * The channel is not allowed for passive scanning or active (if country IE was received and channel is not in the regulatory domain) OR
         * Scan type was set to an invalid value THEN
         * remove it from the channels list */
        if ((status != TI_OK) ||
              (TI_FALSE == pParam->content.channelCapability.channelValidityPassive) ||
              (pPeriodicScanParams->tChannels[i].eScanType != SCAN_TYPE_NORMAL_PASSIVE && pPeriodicScanParams->tChannels[i].eScanType != SCAN_TYPE_NORMAL_ACTIVE))
        {
            if ((pPeriodicScanParams->tChannels[i].eScanType != SCAN_TYPE_NORMAL_PASSIVE && pPeriodicScanParams->tChannels[i].eScanType != SCAN_TYPE_NORMAL_ACTIVE))
            {
                TRACE2(pScanCncn->hRegulatoryDomain, REPORT_SEVERITY_WARNING, "scanCncn_PeriodicVerifyChannelsWithRegDomain: Channel %d discarded since scanType = %d\n", pPeriodicScanParams->tChannels[i].uChannel, pPeriodicScanParams->tChannels[i].eScanType);
            }
            if (i < uChannelNum - 1)
            {
                TI_UINT32 j;
                /* channel not allowed - copy the rest of the channel in its place */
                /* use a loop to copy and not memCopy (which depends on OS implementation) due to overlapping memory copy */
                for (j = i; j < uChannelNum - 1; j++ )
                {
                    os_memoryCopy (pScanCncn->hOS, &(pPeriodicScanParams->tChannels[ j ]),
                               &(pPeriodicScanParams->tChannels[ j+1 ]),
                               sizeof(TPeriodicChannelEntry));

                }
            }
            pPeriodicScanParams->uChannelNum--;
            uChannelNum--;
            continue;
        }

        /* if channel was requested for passive scanning and passive is allowed continue to the next channel */
        if (TI_TRUE == pParam->content.channelCapability.channelValidityPassive &&
                pPeriodicScanParams->tChannels[i].eScanType == SCAN_TYPE_NORMAL_PASSIVE)
        {
            i++;
            continue;
        }

        /* if channel was requested for active scanning and active is allowed set the max tx power */
        if (pPeriodicScanParams->tChannels[i].eScanType == SCAN_TYPE_NORMAL_ACTIVE)
        {
            TI_UINT32 timeDiff = os_timeStampMs(pScanCncn->hOS)-(pParam->content.channelCapability.timestamp);
            /* Check if channel is allowed in the regulatory domain according to the country IE or
             * If a AP was seen in the channel in the last 10 seconds */
            if (TI_TRUE == pParam->content.channelCapability.channelValidityActive &&
                 (pParam->content.channelCapability.bChanneInCountryIe == TI_TRUE || timeDiff < CHANNEL_VALIDITY_TS_THRESHOLD))
            {
                pPeriodicScanParams->tChannels[ i ].uTxPowerLevelDbm =
                                    TI_MIN( pParam->content.channelCapability.uMaxTxPowerDomain,
                                            pPeriodicScanParams->tChannels[ i ].uTxPowerLevelDbm );
            }
            else /* If channel is not allowed for active change it for passive scan */
            {
                pPeriodicScanParams->tChannels[ i ].eScanType = SCAN_TYPE_NORMAL_PASSIVE;
                pPeriodicScanParams->tChannels[ i ].uMaxDwellTimeMs = DEFAULT_MAX_DWELL_TIME_PASSIVE;
                pPeriodicScanParams->tChannels[ i ].uMinDwellTimeMs = DEFAULT_MIN_DWELL_TIME_PASSIVE;
            }

            i++;
            continue;
        }

        TRACE3(pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_PeriodicVerifyChannelsWithRegDomain: OOPS channel %d did not fall in any case, index = %x, ScanType = %d\n", channel, i, pPeriodicScanParams->tChannels[i].eScanType);
        i++;
    }

    os_memoryFree(pScanCncn->hOS, pParam, sizeof(paramInfo_t));
    return TI_OK;
}

/** 
 * \fn     scanCncn_SGconfigureScanParams
 * \brief  Configures Bluetooth coexistence compensation paramters
 * 
 * Configures Bluetooth coexistence compensation paramters.
 * This function is called when SG is enabled or disabled from the SoftGemini module.
 *          The compensation is needed since BT Activity holds the antenna and over-ride Scan activity
 *
 * \param hScanCncn - handle to the scan concentrator object.\n
 * \param bUseSGParams - whether to use the new parameters (TI_TRUE when SG is enabled)
 * \param  probeReqPercent - increasing num probe requests in that percentage
 * \param SGcompensationMaxTime - max value from which we won't increase dwelling time
 * \param SGcompensationPercent - increasing dwell time in that percentage
 * \return None
 * \sa     scanCncn_SGupdateScanParams
 */ 
void scanCncn_SGconfigureScanParams (TI_HANDLE hScanCncn, TI_BOOL bUseSGParams,
                                     TI_UINT8 probeReqPercent, TI_UINT32 SGcompensationMaxTime, 
                                             TI_UINT32 SGcompensationPercent) 
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;

    pScanCncn->bUseSGParams             = bUseSGParams;
    pScanCncn->uSGprobeRequestPercent   = probeReqPercent;
    pScanCncn->uSGcompensationMaxTime   = SGcompensationMaxTime;
    pScanCncn->uSGcompensationPercent   = SGcompensationPercent;

    TRACE4(pScanCncn->hReport, REPORT_SEVERITY_INFORMATION , "scanCncn_SGconfigureScanParams: bUseSGParams=%d, numOfProbeRequest=%d, compensationMaxTime=%d, SGcompensationPercent=%d\n", pScanCncn->bUseSGParams, pScanCncn->uSGprobeRequestPercent, pScanCncn->uSGcompensationMaxTime, pScanCncn->uSGcompensationPercent);
}


/** 
 * \fn     scanCncn_SGupdateScanParams
 * \brief  Updates dwell times and probe request number to compensate for bluetooth transmissions
 * 
 * Updates dwell times and probe request number to compensate for bluetooth transmissions
 * 
 * \param  hScanCncn - handle to the scan concentrator object
 * \param  puScanParams - a pointer to the scan parmeters union
 * \param  bPeriodicScan - TRUE if the parameters are for periodic scan, FALSE if for one-shot scan
 * \return None
 * \sa     scanCncn_SGconfigureScanParams
 */ 
void scanCncn_SGupdateScanParams (TI_HANDLE hScanCncn, UScanParams *puScanParams, TI_BOOL bPeriodicScan) 
{
    TScanCncn           *pScanCncn = (TScanCncn*)hScanCncn;
    TI_UINT32           i, uTempTime;

    /* one shot scan */
    if (TI_FALSE == bPeriodicScan)
    {
        /* get a pointer to the one-shot scan params */
        TScanParams *pScanParams = &(puScanParams->tOneShotScanParams);

        /* for each channel increase the min and max dwell time */
        for (i = 0; i < pScanParams->numOfChannels; i++)
        {   
            /* SPS scan */
            if (SCAN_TYPE_SPS == pScanParams->scanType)
            {
                if (pScanCncn->uSGcompensationMaxTime >
                    pScanParams->channelEntry[i].SPSChannelEntry.scanDuration)
                {
                    uTempTime = ((pScanParams->channelEntry[i].SPSChannelEntry.scanDuration) * 
                        (100 + pScanCncn->uSGcompensationPercent)) / 100 ;

                    if (uTempTime > pScanCncn->uSGcompensationMaxTime)
                    {
                        uTempTime = pScanCncn->uSGcompensationMaxTime;
                    }
                    pScanParams->channelEntry[i].SPSChannelEntry.scanDuration = uTempTime;
                }
            }
            /* all other scan types */
            else
            {
                if (pScanCncn->uSGcompensationMaxTime >
            pScanParams->channelEntry[i].normalChannelEntry.minChannelDwellTime)
                {
                    uTempTime = ((pScanParams->channelEntry[i].normalChannelEntry.minChannelDwellTime) * 
                        (100 + pScanCncn->uSGcompensationPercent)) / 100 ;

                    if (uTempTime > pScanCncn->uSGcompensationMaxTime)
                    {
                        uTempTime = pScanCncn->uSGcompensationMaxTime;
                    }
                    pScanParams->channelEntry[i].normalChannelEntry.minChannelDwellTime = uTempTime;
                }

                if (pScanCncn->uSGcompensationMaxTime > 
                pScanParams->channelEntry[i].normalChannelEntry.maxChannelDwellTime)
                {
                    uTempTime = ((pScanParams->channelEntry[i].normalChannelEntry.maxChannelDwellTime) * 
                        (100 + pScanCncn->uSGcompensationPercent)) / 100 ;
                    
                    if (uTempTime > pScanCncn->uSGcompensationMaxTime)
                    {
                        uTempTime = pScanCncn->uSGcompensationMaxTime;
                    }
                    pScanParams->channelEntry[i].normalChannelEntry.maxChannelDwellTime = uTempTime;
                }
            }
        }

        /* update ProbeReqNumber by SG percantage */
        if (pScanParams->probeReqNumber > 0)
        {
            pScanParams->probeReqNumber = ((pScanParams->probeReqNumber) * 
                    (100 + pScanCncn->uSGprobeRequestPercent)) / 100 ;
        }
    }
    /* periodic scan */
    else
    {
        TPeriodicScanParams *pPeriodicScanParams = &(puScanParams->tPeriodicScanParams);

        /* for each channel increase the min and max dwell time */
        for (i = 0; i < pPeriodicScanParams->uChannelNum; i++)
        {   
            if (pScanCncn->uSGcompensationMaxTime >
                pPeriodicScanParams->tChannels[ i ].uMinDwellTimeMs)
            {
                uTempTime = ((pPeriodicScanParams->tChannels[ i ].uMinDwellTimeMs) * 
                    (100 + pScanCncn->uSGcompensationPercent)) / 100 ;

                if (uTempTime > pScanCncn->uSGcompensationMaxTime)
                {
                    uTempTime = pScanCncn->uSGcompensationMaxTime;
                }
                pPeriodicScanParams->tChannels[ i ].uMinDwellTimeMs = uTempTime;
            }

            if (pScanCncn->uSGcompensationMaxTime >
                pPeriodicScanParams->tChannels[ i ].uMaxDwellTimeMs)
            {
                uTempTime = ((pPeriodicScanParams->tChannels[ i ].uMaxDwellTimeMs) * 
                    (100 + pScanCncn->uSGcompensationPercent)) / 100 ;

                if (uTempTime > pScanCncn->uSGcompensationMaxTime)
                {
                    uTempTime = pScanCncn->uSGcompensationMaxTime;
                }
                pPeriodicScanParams->tChannels[ i ].uMaxDwellTimeMs = uTempTime;
            }
        }

        /* update ProbeReqNumber by SG percantage */
        if (pPeriodicScanParams->uProbeRequestNum > 0)
        {
            pPeriodicScanParams->uProbeRequestNum = ((pPeriodicScanParams->uProbeRequestNum) * 
                    (100 + pScanCncn->uSGprobeRequestPercent)) / 100 ;
        }
    }
}

/** 
 * \fn     scanCncn_Mix1ShotScanChannels
 * \brief  Mix the channel order in a 1 Shot Scan channel array.
 * 
 * Mix the channel order in a 1 Shot Scan channel array.
 * 
 * \param  pChannelArray - where to store allowed channels information
 * \param  uValidChannelsCount - Number of allowed channels (that were placed in the given channels array)
 * \return None
 */ 
static void scanCncn_Mix1ShotScanChannels (TScanChannelEntry *pChannelArray, TI_UINT32 uValidChannelsCount)
{
    TI_UINT32 i;
    TScanChannelEntry tTempArray[MAX_CHANNEL_IN_BAND_2_4];   

    if (uValidChannelsCount <= MIN_CHANNEL_IN_BAND_2_4)
    {
        return;
    }

    if (uValidChannelsCount > MAX_CHANNEL_IN_BAND_2_4)
    {
        uValidChannelsCount = MAX_CHANNEL_IN_BAND_2_4;
    }

    /*
     * Create new Channels Array that will mix the channels
     * something like: 1,8,2,9,3,10,4,11,5,12,6,13,7,14
     * For odd number of channels, the last channel will be adjacent, but never mind...
     */
    for (i = 0 ; i < uValidChannelsCount - 1 ; i+=2)
    {
        tTempArray[i] = pChannelArray[i/2];
        tTempArray[i+1] = pChannelArray[(uValidChannelsCount+i)/2];
    }

    /* if this is the last odd channel */
    if ((i + 1)  == uValidChannelsCount)
    {
        tTempArray[i] = tTempArray[i - 1];
        tTempArray[i - 1] = pChannelArray[uValidChannelsCount - 1];
    }

    /* Now copy to the real array */
    for (i = 0 ; i < uValidChannelsCount; i++)
    {
        pChannelArray[i] = tTempArray[i];
    }
}


/** 
 * \fn     scanCncn_MixPeriodicScanChannels
 * \brief  Mix the channel order in a Periodic Scan channel
 * array.
 * 
 * Mix the channel order in a Periodic Scan channel array.
 * 
 * \param  pChannelArray - where to store allowed channels information
 * \param  uValidChannelsCount - Number of allowed channels (that were placed in the given channels array)
 * \return None
 */ 
static void scanCncn_MixPeriodicScanChannels (TPeriodicChannelEntry *pChannelArray, TI_UINT32 uValidChannelsCount)
{
    TI_UINT32 i;
    TPeriodicChannelEntry tTempArray[MAX_CHANNEL_IN_BAND_2_4];  

    if (uValidChannelsCount <= MIN_CHANNEL_IN_BAND_2_4)
    {
        return;
    }

    if (uValidChannelsCount > MAX_CHANNEL_IN_BAND_2_4)
    {
        uValidChannelsCount = MAX_CHANNEL_IN_BAND_2_4;
    }

    /*
     * Create new Channels Array that will mix the channels
     * something like: 1,8,2,9,3,10,4,11,5,12,6,13,7,14
     * For odd number of channels, the last channel will be adjacent, but never mind...
     */
    for (i = 0 ; i < uValidChannelsCount - 1 ; i+=2)
    {
        tTempArray[i] = pChannelArray[i/2];
        tTempArray[i+1] = pChannelArray[(uValidChannelsCount+i)/2];
    }

    /* if this is the last odd channel */
    if ((i + 1)  == uValidChannelsCount)
    {
        tTempArray[i] = tTempArray[i - 1];
        tTempArray[i - 1] = pChannelArray[uValidChannelsCount - 1];
    }

    /* Now copy to the real array */
    for (i = 0 ; i < uValidChannelsCount; i++)
    {
        pChannelArray[i] = tTempArray[i];
    }
}

/** 
 * \fn     scanCncn_ScanCompleteCB
 * \brief  CB to be called upon ScanComplete event reception.
 * 
 * \param hScanCncn - handle to the scan concentrator object.
 * \param  str      - Buffer containing the event result
 * \param  strLen   - Event result length
 * 
 * \return None
 */ 
static void scanCncn_ScanCompleteCB( TI_HANDLE hScanCncn, char* str, TI_UINT32 strLen )
{
	TScanCncn *pScanCncn = (TScanCncn*)hScanCncn;
    scanCompleteResults_t *pResult = (scanCompleteResults_t*)str;
	EScanResultTag      eScanTag;
	TI_UINT32           uResultCount = 0;
	TI_BOOL             bTSFError = TI_FALSE;
	TI_UINT16           SPSScanResult = 0xffff;

	/* copy scan tag */
	eScanTag = (EScanResultTag)pResult->scanTag;

    TRACE1( pScanCncn->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_ScanCompleteCB - Scan complete event received for client with scan tag %d\n", eScanTag);

	/* nullify the consecutive no scan complete events counter  - only if this is a scan complete that
       does not happen afetr a stop scan (due to a timer expiry) */
	if ( TI_TRUE == pScanCncn->bScanCompleteFlag )
    {
		pScanCncn->numOfConsTimerExpiry = 0;
    }

#if 0
	if (eScanTag==0) { /* Sanity check, this cannot happen TODO Check with TI R&D */
		TRACE1( pScanCncn->hReport, REPORT_SEVERITY_ERROR, "scanCncn_ScanCompleteCB - Scan complete event received for client with scan tag WORKAROUND %d\n", eScanTag);
		eScanTag = 1;
	}
#endif
    /* Check Result of Scan to distinguish PS failure*/
	if (SCHEDULED_SCAN_COMPLETED_OK != pResult->scheduledScanStatus)
	{
		/* Failed to enter PS - Scan didn't start*/
		return scanCncn_ScanCompleteNotification (hScanCncn, eScanTag, uResultCount,
												  SPSScanResult, bTSFError, TI_NOK);
	}

    /* copy result counter */

    uResultCount = pResult->numberOfScanResults;


    /* copy scan SPS addmitted channels and SPS result */
    if (TI_FALSE == pScanCncn->eLatestScanType) 
    {
		/* normal scan - no result is available */
        bTSFError = TI_FALSE;
        TRACE0( pScanCncn->hReport, REPORT_SEVERITY_INFORMATION, "Normal scan completed.\n");
    }
    else
    {
        /* SPS scan - first byte indicates whether a TSF error (AP recovery) occured */
        if ( 0 != (pResult->scheduledScanStatus >> 24))
        {
            bTSFError = TI_TRUE;
        }
        else
        {
			bTSFError = TI_FALSE;
        }

        /* next two bytes indicates on which channels scan was attempted */
        SPSScanResult = (TI_UINT16)(pResult->scheduledScanStatus >> 16) | 0xff;
        SPSScanResult = ENDIAN_HANDLE_WORD( SPSScanResult );
        TRACE1( pScanCncn->hReport, REPORT_SEVERITY_INFORMATION, "SPS scan completed. TSF error: , SPS result: %x\n", SPSScanResult);
    }

	scanCncn_ScanCompleteNotification (hScanCncn, eScanTag, uResultCount, SPSScanResult,
									   bTSFError, TI_OK);
}

/*
 * \brief	notifies the ScanCncn one of its clients has stopped
 *
 * 			Invoked by the client after it was stopped.
 * 			Notifies the pwrState module when all clients have stopped.
 *
 * \param	hScanCncn		this module
 * \param	hScanCncnClient	the client that was stopped
 *
 */
void scanCncn_ClientStopped(TI_HANDLE hScanCncn, EScanCncnClient eClient)
{
	TScanCncn *this = (TScanCncn*)hScanCncn;
	EScanCncnClient  i;

	/* don't notify pwrState if a client is still running */
	for (i = 0; i < SCAN_SCC_NUM_OF_CLIENTS; ++i)
	{
		if (this->pScanClients[i]->bCurrentlyRunning)
		{
			TRACE2(this->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_ClientStopped: client %d has stopped. still waiting for client %d\n", eClient, i);
			return;
		}
	}

	/* when all clients have stopped - notify pwrState */
	TRACE1(this->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_ClientStopped: last client (%d) has stopped\n", eClient);
	pwrState_ScanCncnStopped(this->hPwrState);
}

/*
 * \brief	Reverts scanCncn_Suspend()'s actions - restarts any scan-clients
 * 			that were stopped
 *
 * \param	hScanCncn	this module
 *
 * \return	TI_OK
 */
TI_STATUS scanCncn_Resume(TI_HANDLE hScanCncn)
{
	TScanCncn       *this = (TScanCncn*)hScanCncn;
	EScanCncnClient  eClient;

	for (eClient = 0; eClient < SCAN_SCC_NUM_OF_CLIENTS; ++eClient)
	{
		if (this->pScanClients[eClient]->bSuspended)
	{
			TRACE1(this->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_Resume: resuming client %d\n", eClient);

			this->pScanClients[eClient]->bSuspended = TI_FALSE; /* allow scan requests */

			switch (eClient)
			{
			case SCAN_SCC_APP_PERIODIC:
				scanCncn_StartPeriodicScan(this, eClient, &this->pScanClients[eClient]->uScanParams.tPeriodicScanParams);
				break;
			case SCAN_SCC_APP_ONE_SHOT:
				scanCncn_Start1ShotScan(this, eClient, &this->pScanClients[eClient]->uScanParams.tOneShotScanParams);
				break;
			case SCAN_SCC_ROAMING_CONT: /* will be started by ScanMngr */
			case SCAN_SCC_ROAMING_IMMED: /* will be started by ScanMngr */
			case SCAN_SCC_DRIVER: /* will be started by SME */
			default:
				break;
	}


		}
	}

	return TI_OK;
}

/*
 * \brief	Suspends the ScanCncn module
 *
 * 			Stops any ongoing scans
 *
 * \param	hScanCncn	this module
 *
 * \return	TI_OK if all clients are stopped;
 * 			TI_PENDING if clients were told to stop (see scanCncn_ClientStopped)
 */
TI_STATUS scanCncn_Suspend(TI_HANDLE hScanCncn)
{
	TScanCncn       *this = (TScanCncn*)hScanCncn;
	EScanCncnClient  eClient;
	TI_BOOL          bWaitForClients = TI_FALSE;

	for (eClient = 0; eClient < SCAN_SCC_NUM_OF_CLIENTS; ++eClient)
	{
		if (this->pScanClients[eClient]->bCurrentlyRunning)
	{
			TRACE1(this->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_Suspend: suspending client %d\n", eClient);

			this->pScanClients[eClient]->bSuspended = TI_TRUE; /* remember to resume it; reject scans until resumed; enable short StopScan timeout */

			scanCncn_StopScan(this, eClient); /* stop the client */


			bWaitForClients = TI_TRUE;
	}
	}

	if (! bWaitForClients)
	{
		TRACE0(this->hReport, REPORT_SEVERITY_INFORMATION, "scanCncn_Suspend: nothing to do\n");

		pwrState_ScanCncnStopped(this->hPwrState);
		return TI_OK;
	}

	return TI_PENDING;
}





