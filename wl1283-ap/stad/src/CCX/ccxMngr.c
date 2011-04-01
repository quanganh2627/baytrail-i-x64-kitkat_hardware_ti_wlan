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
/** \file ccxMngr.c
 *  \brief CCX (Cisco Compatible Extensions for WLAN Devices) Manager
 *
 *  \see ccxMngr.h
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  CCX Manager                                                   *
 *   PURPOSE: 
                The Cisco Compatible eXtensions (CCX) Manager module 
                supports interface for all CCX proprietary requirements.
                This includes:
                - Rogue AP report and log
                - Parse and build IAPP (Inter AP Protocol) packets
                - Handle CCKM: existence, OIDs and events IF with the Supplicant
                - Parse and build CCX version number IE 
                - Cisco List of APs and channels                            
 *                                                                          *
 ****************************************************************************/
#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_11
#include "tidef.h"
#include "osApi.h"
#include "paramOut.h"
#include "report.h"
#include "qosMngr_API.h"
#include "rsnApi.h"
#include "ccxMngr.h"
#include "rogueAp.h"
#include "802_11Defs.h"
#include "DataCtrl_Api.h"
#include "timer.h"
#include "rsn.h"
#include "siteMgrApi.h"
#include "EvHandler.h"
#include "TI_IPC_Api.h"
#include "apConn.h"
#include "TWDriver.h"
#include "ccxTSMngr.h"
#include "DrvMainModules.h"
#include "smeApi.h"
#include "RxBuf.h"

/* Constants */
extern int WMEQosTagToACTable[MAX_NUM_OF_802_1d_TAGS];

#define CCX_CCKM_DELAY_MS           5000   /* 5 sec */

/* Nominal Rate requested - 6 Mbits/sec in units of 500 kbit/s*/
#define WME_TSPEC_DEFAULT_NOMINAL_RATE  (12)


#define CCKM_REUATH_RETRIES_LIMIT           2

/* Enumerations */

/* Typedefs */

/* Structures */
/* CCX version number IE definitions as it appears in CCX V2 S38 */
#define CCX_VERSION_NUMBER_LEN      0x05

typedef struct 
{
    TI_UINT8 elementid;            /*< CCX Version Num IE element ID - should always be 0xDD */
    TI_UINT8 length;               /*< CCX Version Num  IE length - 5 */
    TI_UINT8 oui[3];               
    TI_UINT8 VerNumType;           /*< CCX Version Num  type - 3 */
    TI_UINT8 VerNum;               /*< CCX Version Num  - 2 or 3 */
} ccxVersionNumIePacket_t;

/* CCX IAPP Information packet definitions as it appears in CCX V2 S32 */
#define IAPP_FUNCTION_TYPE    0x00

#define IAPP_REPORT_INFORMATION_LENGTH      85  /*(sizeof(IAPP_Report_Information_msg_t))*/
#define TAG_ADJACENT_AP_REPORT              0x9b
#define IAPP_REPORT_INFORMATION_BODY_LENGTH 48

#define TAG_ASSOC_ROAM_REPORT               0x9c
#define IAPP_ASSOC_ROAM_LENGTH				5   /*(sizeof(IAPP_AssocRoam_msg_body_t))*/

typedef struct 
{
    TI_UINT16              tag_adjacentApReport;
    TI_UINT16              length;
    TI_UINT8               aironetOui[4];
    TMacAddr               previousApMac;
    TI_UINT16              channel;
    TI_UINT16              ssidLength;
    TI_UINT8               ssid[32];
    TI_UINT16              disassociateTime;
} IAPP_Report_Information_msg_body_t;

typedef struct 
{
    TI_UINT16              tag_assocRoam;
    TI_UINT16              length;
    TI_UINT8               aironetOui[4];
    TI_UINT8               associationReasonCode;
} IAPP_AssocRoam_msg_body_t;

typedef struct 
{
    IAPP_msg_header_t                   IAPP_Report_Information_header;
    IAPP_Report_Information_msg_body_t  IAPP_Report_Information_msg_body[1]; /* Variable length, max MAX_AP_INFORMATION_REPORT_LIST*/
	IAPP_AssocRoam_msg_body_t			IAPP_AssocRoam_msg_body;
} IAPP_Report_Information_msg_t;

typedef enum
{
/*	0	*/	ASSOC_REASON_UNSPECIFIED,						/** <Unspecfied> */		

/*	1	*/	ASSOC_REASON_NORMAL_POOR_LINK,					/**< Normal roam, poor link (excessive retries, 
																	too much interference, RSSI too low, etc.)	*/
/*	2	*/	ASSOC_REASON_NORMAL_LOAD_BALANCING,				/**< Normal roam, Load balancing */
/*	3	*/	ASSOC_REASON_CAPACITY_TSPEC_REJECTED,			/**< AP has insufficient capacity (TSPEC rejected) */
/*	4	*/	ASSOC_REASON_INFRASTRUCTURE_DIRECTED,			/**< Infrastructure directed roam */
/*	5	*/	ASSOC_REASON_FIRST_ASSOC_TO_WLAN,				/**< First association to WLAN */
/*	6	*/	ASSOC_REASON_ROAMMING_IN,						/**< Roaming in from cellular or other WAN */
/*	7	*/	ASSOC_REASON_ROAMMING_OUT,						/**< Roaming out to cellular or other WAN */
/*	8	*/	ASSOC_REASON_NORMAL_ROAM_BETTER_AP,				/**< Normal roam, better AP found */
/*	9	*/	ASSOC_REASON_DEAUTH_FROM_AP,					/**< Deauthenticated or Disassociated from the previous AP */
} ccx_assocReasonCode_e;

/* External data definitions */

/* Local functions definitions */

/* Global variables */

/* Function prototypes */
static void ccxMngr_cckmTimerExpired(TI_HANDLE hCcxMngr, TI_BOOL bTwdInitOccured);
static TI_STATUS ccxMngr_recvIappInformationReport(TI_HANDLE hCcxMngr, TI_UINT8 *iappPacket);
static void  ccxMngr_updatePrevAssocParams(ccxMngr_t *pCcxMngr);
static void ccxMngr_saveNeighborAps(ccxMngr_t *pCcxMngr, IAPP_Report_Information_msg_body_t  *pIAPP_Report_Information_msg_body);
static TI_STATUS ccxMngr_reportCcxEnableStatusChange(ccxMngr_t *pCcxMngr);
static void ccxMngr_buildTsrsIE(TI_UINT8 tsid, TI_UINT8 *pDataBuff, TI_UINT32 *len);
static TI_STATUS ccxMngr_LinkTestResponse(TI_HANDLE hMeasurement, TI_UINT8 *iappPacket);
static TI_UINT8 ccxMngr_translateAssocRoamingTriggerToReasonCode(ccxMngr_t *pCcxMngr, apConn_roamingTrigger_e roamingTrigger);

/*****************************************************************************
 **         Public Function section                                      **
 *****************************************************************************/

/**
*
* ccxMngr_create
*
* \b Description: 
*
* Create the CCX Manager context.
*
* \b ARGS:
*
*  I   - hOs - OS handler  \n
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure.
*
* \sa 
*/
TI_HANDLE ccxMngr_create(TI_HANDLE hOs)
{
    ccxMngr_t *pCcxMngr;

    /* allocate context memory */
    pCcxMngr = (ccxMngr_t*)os_memoryAlloc(hOs, sizeof(ccxMngr_t));
    if (pCcxMngr == NULL)
    {
        return NULL;
    }

    os_memoryZero(hOs, (TI_HANDLE)pCcxMngr, sizeof(ccxMngr_t));

    pCcxMngr->hOs = hOs;

    pCcxMngr->hRogueAp = rogueAp_create(hOs);
    if (pCcxMngr->hRogueAp == NULL)
    {
        os_memoryFree(hOs, (TI_HANDLE)pCcxMngr, sizeof(ccxMngr_t));
        return NULL;
    }
     
    return (TI_HANDLE)pCcxMngr;
}

/**
*
* ccxMngr_unload
*
* \b Description: 
*
* Unload CCX Manager module from memory
*
* \b ARGS:
*
*  I   - hAdmCtrl - CCX Manager context  \n
*
* \b RETURNS:
*
*  TI_OK if successful, TI_NOK otherwise.
*
* \sa ccxMngr_create
*/
TI_STATUS ccxMngr_unload(TI_HANDLE hCcxMngr)
{
    TI_STATUS       status;
    ccxMngr_t       *pCcxMngr;
    
    if (hCcxMngr == NULL)
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

	if (pCcxMngr->hCckmTimer)
	{
		tmr_DestroyTimer (pCcxMngr->hCckmTimer);
	}

    status = rogueAp_unload(pCcxMngr->hRogueAp);
    if (status!=TI_OK)
    {
        return status;
    }

    os_memoryFree(pCcxMngr->hOs, hCcxMngr, sizeof(ccxMngr_t));

    return TI_OK;
}

/**
*
* ccxMngr_config
*
* \b Description: 
*
* Configure the CCX Manager module.
*
* \b ARGS:
*
*  I   - pStadHandles  - The driver modules handles \n
*  
* \b RETURNS:
*
*  void
*
* \sa 
*/
void ccxMngr_init (TStadHandlesList *pStadHandles)   
{
    ccxMngr_t *pCcxMngr = (ccxMngr_t*)(pStadHandles->hCcxMngr);
    iappParsingRegistrationTable_t  iappParsingRegistration;
    TI_STATUS   status;

    /* Initialize CCX Manager handlers */
    pCcxMngr->hReport           = pStadHandles->hReport;
    pCcxMngr->hRsn              = pStadHandles->hRsn;
    pCcxMngr->hCtrlData         = pStadHandles->hCtrlData;
    pCcxMngr->hTxCtrl           = pStadHandles->hTxCtrl;
    pCcxMngr->hTxMgmtQ          = pStadHandles->hTxMgmtQ;
    pCcxMngr->hSiteMgr          = pStadHandles->hSiteMgr;
    pCcxMngr->hSme              = pStadHandles->hSme;
    pCcxMngr->hAPConn           = pStadHandles->hAPConnection;
    pCcxMngr->hEvHandler        = pStadHandles->hEvHandler; 
    pCcxMngr->hMeasurementMngr  = pStadHandles->hMeasurementMgr;
    pCcxMngr->hQosMngr          = pStadHandles->hQosMngr;
    pCcxMngr->hTimer            = pStadHandles->hTimer;
    pCcxMngr->hTWD              = pStadHandles->hTWD;

    /* Make sure the struct above is compiled without alignment gaps (as it's sent in a frame). */
    if ( sizeof(IAPP_Report_Information_msg_t) != IAPP_REPORT_INFORMATION_LENGTH )
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "sizeof(IAPP_Report_Information_msg_body_t) is %d instead of 76!! (should be packed without gaps in it)\n", sizeof(IAPP_Report_Information_msg_body_t));
    }

    /* Configure Rogue AP module */
    status = rogueAp_config(pCcxMngr->hRogueAp, pCcxMngr->hReport, pCcxMngr->hOs, (TI_HANDLE)pCcxMngr);

    /* Register a procedure to receive the IAPP Information packet after association */
    iappParsingRegistration.handler = (TI_HANDLE)pCcxMngr;
    iappParsingRegistration.iappParsingRegistrationProcedure = ccxMngr_recvIappInformationReport;
    ccxMngr_registerForRecvIappPacket((TI_HANDLE)pCcxMngr, iappParsingRegistration, IAPP_REPORT_INFORMATION);
    
    iappParsingRegistration.iappParsingRegistrationProcedure = ccxMngr_LinkTestResponse;
    ccxMngr_registerForRecvIappPacket((TI_HANDLE)pCcxMngr, iappParsingRegistration, IAPP_LINK_TEST);
}


TI_STATUS ccxMngr_SetDefaults (TI_HANDLE hCcxMngr, ccxMngrParams_t *pCcxMngrParams)
{
    ccxMngr_t *pCcxMngr = (ccxMngr_t*)hCcxMngr;

    /* Initialize CCX Manager internal variables */
    pCcxMngr->ccxEnable                     = pCcxMngrParams->ccxEnabled;
    pCcxMngr->ccxConfiguration              = 0;           
    pCcxMngr->sendIappInfo                  = TI_FALSE;                 
    pCcxMngr->useCckmFastRoaming            = TI_FALSE;         
    pCcxMngr->ccxSupportedVersion           = 4;   /* Currently supporting CCX version 4 */ 
    pCcxMngr->apCandidateCcxVersion         = 0;
    pCcxMngr->cckmAkmExists                 = TI_FALSE; 
    pCcxMngr->useCckmAkm                    = TI_TRUE;
    pCcxMngr->disAssociateTime              = 0; 
                                
    pCcxMngr->associationRequestIELength    = 0; 
    pCcxMngr->IappSentInfoTS                = 0;

    pCcxMngr->neighborAPList.numOfEntries = 0;
    os_memoryZero(pCcxMngr->hOs, pCcxMngr->neighborAPList.APListPtr, sizeof(neighborAP_t)*MAX_AP_INFORMATION_REPORT_LIST);

    /* allocate OS timer memory */
    pCcxMngr->hCckmTimer = tmr_CreateTimer (pCcxMngr->hTimer);
    if (pCcxMngr->hCckmTimer == NULL)
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_ERROR , "ccxMngr_SetDefaults(): Failed to create hCckmTimer!\n");
        return TI_NOK;
    }

    return TI_OK;
}


/**
*
* ccxMngr_setParam - Set a specific parameter to the ccxMngr SM
*
* \b Description: 
*
* Set a specific parameter to the ccxMngr SM.
*
* \b ARGS:
*
*  I   - hCcxMngr - ccxMngr SM context  \n
*  I/O - pParam - Parameter \n
*
* \b RETURNS:
*
*  TI_OK if successful, TI_NOK otherwise.
*
* 
*/
TI_STATUS ccxMngr_setParam(TI_HANDLE hCcxMngr, paramInfo_t *pParam)
{
    ccxMngr_t           *pCcxMngr;
    TI_STATUS           status=TI_OK;

    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    if ((hCcxMngr == NULL) || (pParam == NULL))
    {
        return TI_NOK;
    }

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam   %X \n", pParam->paramType);

    if ((pCcxMngr->ccxEnable!=CCX_MODE_ENABLED) && (pParam->paramType!=CCX_ENABLED))
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam when CCX is disabled \n");
        return TI_NOK;
    }

    switch (pParam->paramType)
    {
    case CCX_CONFIGURATION:
        /* CCX configuration includes: RogueAP, CCKM and CKIP */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_CONFIGURATION=  %X \n", pParam->content.ccxConfiguration);

        pCcxMngr->ccxConfiguration = pParam->content.ccxConfiguration;
        pCcxMngr->useCckmAkm = TI_TRUE;
        break;    
    
    case CCX_AUTH_SUCCESS:
        {   /* Since authnetication success arrives from RSN, when Keys are received,
            this OID is not used.*/
            TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_AUTH_SUCCESS \n");
        }
        break;

    case CCX_ROGUE_AP_DETECTED:
        {
            paramInfo_t param;
            /* Informs that Rogue AP was detected only in LEAP auth */
            TRACE7(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_ROGUE_AP_DETECTED, MAC =%x-%x-%x-%x-%x-%x , reason=%d\n", pParam->content.rogueApDetected.RogueAPMacAddress[0], pParam->content.rogueApDetected.RogueAPMacAddress[1], pParam->content.rogueApDetected.RogueAPMacAddress[2], pParam->content.rogueApDetected.RogueAPMacAddress[3], pParam->content.rogueApDetected.RogueAPMacAddress[4], pParam->content.rogueApDetected.RogueAPMacAddress[5], pParam->content.rogueApDetected.FailureReason);
    
            param.paramType = RSN_CCX_NETWORK_EAP;
            rsn_getParam(pCcxMngr->hRsn, &param);
    
            if ((pCcxMngr->ccxConfiguration & OS_CCX_CONFIGURATION_ENABLE_ROGUE_AP) &&
                (param.content.networkEap != OS_CCX_NETWORK_EAP_OFF))
            {   /* Add to Rogue AP DB */
                status = rogueAp_addRogueAp(pCcxMngr->hRogueAp, &pParam->content.rogueApDetected);
                /* Indicate that the assoc failed due to Rogue AP */
                ccxMngr_updateIappInformation(pCcxMngr, CCX_ROGUE);
            }

            {
                /* Raise the EAP-Failure as event */
                TI_UINT16   *pEventData = (TI_UINT16 *)&pParam->content.rogueApDetected.FailureReason;
                (*pEventData) &= 0x00FF;
                EvHandlerSendEvent(pCcxMngr->hEvHandler, IPC_EVENT_EAP_AUTH_FAILURE, (TI_UINT8*)pEventData, sizeof(TI_UINT16));
            }
        }
        break;
    case CCX_REPORT_ROGUE_APS:
        /* Upon successful association, report Rogue APs */
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_REPORT_ROGUE_APS - Do nothing!!\n");
        /* This will be handled when ASSOC TI_OK arrives,
        in order to prevent receiving this IOCTL before all keys are set in FW. */
        /*if (pCcxMngr->ccxConfiguration & OS_CCX_CONFIGURATION_ENABLE_ROGUE_AP)
        {
            status = rogueAp_report(pCcxMngr->hRogueAp, pParam->content.reportRogueAp);
        }*/
        break; 

    case CCX_CCKM_REQUEST:
        /* This is set by the suppliacnt in response to event: 
            CCKM_START. It indicates whether CCKM fast off is used or not. */
        /* stop the timer for case that the CCKM_REQUEST did not arrive */
        tmr_StopTimer (pCcxMngr->hCckmTimer);
        /* get the CCKM result */
        pCcxMngr->useCckmFastRoaming = (pParam->content.ccxCckmRequest.RequestCode == Ccx_CckmFastHandoff) ? TI_TRUE : TI_FALSE;
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_CCKM_REQUEST, useCckm=%d \n", pCcxMngr->useCckmFastRoaming);
        
        if (pCcxMngr->useCckmFastRoaming)
        {   /* if fast handoff is used, save the CCKM IE for the Re-Assoc REQ */
            if (pParam->content.ccxCckmRequest.AssociationRequestIELength > 255)
            {
                TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_setParam   CCX_CCKM_REQUEST, AssociationRequestIELength = %d \n", pParam->content.ccxCckmRequest.AssociationRequestIELength);
                return TI_NOK;
            }
            os_memoryCopy(pCcxMngr->hOs, pCcxMngr->associationRequestIE, (void*)pParam->content.ccxCckmRequest.AssociationRequestIE, pParam->content.ccxCckmRequest.AssociationRequestIELength); 
            pCcxMngr->associationRequestIELength = (TI_UINT8)pParam->content.ccxCckmRequest.AssociationRequestIELength;
        }
        else
        {   /* FAST handoff is not used. */
            pCcxMngr->associationRequestIELength = 0;
        }
        /* indicate Roaming Manager, that Roaming reply was received */
        apConn_RoamHandoffFinished(pCcxMngr->hAPConn);

        break;     
    case CCX_CCKM_RESULT: 
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam CCX_CCKM_RESULT \n");

        /* The supplicant issues it when successful assoc with CCKM occured.*/
        if (pParam->content.ccxCckmResult.ResultCode == osCcx_CckmFailure)
        {   /* in case of failure, report it to RSN */
            status = rsn_reportStatus((rsn_t *)pCcxMngr->hRsn, TI_NOK);
        }
        /* in case of - osCcx_CckmNotInUse or osCcx_CckmSuccess,
        do nothing. When the keys are derived the station will become
        CONNECTED and the port will be opened for data */
        
        break; 

    case CCX_CCKM_EXISTS:
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam, ccxCckmExists=%x \n",pParam->content.ccxCckmExists);

        /* returns the CCKM existence according to the CCKM AKM existence in the Beacon.
        This is called in the selection of a new AP */
        pCcxMngr->cckmAkmExists = pParam->content.ccxCckmExists;
        break;

    case CCX_ENABLED:
        /* Enable/Disable CCX features */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam, CCX_ENABLED=%d \n",pParam->content.ccxEnabled);
        if (pCcxMngr->ccxEnable != pParam->content.ccxEnabled)
        {
            pCcxMngr->ccxEnable = (ccxMngr_mode_t)pParam->content.ccxEnabled;
            ccxMngr_reportCcxEnableStatusChange(pCcxMngr);
        }
        break;
#if 0
    case CCX_NEIGHBOR_APS:
        /* Sets the desired list of channel for the scan for Roaming */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setParam, channel number=%d \n",pParam->content.channel);

        /*ccxMngr_saveNeighborAps(pCcxMngr, pParam->content.channel);*/

        break;
#endif   
    default:
        return TI_NOK;
    }

    return status;
}

/**
*
* ccxMngr_getParam - Get a specific parameter from the ccxMngr SM
*
* \b Description: 
*
* Get a specific parameter from the ccxMngr SM.
*
* \b ARGS:
*
*  I   - hCcxMngr - ccxMngr object handler  \n
*  I/O - pParam - Parameter \n
*
* \b RETURNS:
*
*  TI_OK if successful, TI_NOK otherwise.
*
* 
*/
TI_STATUS ccxMngr_getParam(TI_HANDLE hCcxMngr, paramInfo_t *pParam)
{
    ccxMngr_t           *pCcxMngr;
    TI_STATUS           status=TI_OK;

    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    if ((hCcxMngr == NULL) || (pParam == NULL))
    {
        return TI_NOK;
    }

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam   %X \n", pParam->paramType);
    
    if ((pCcxMngr->ccxEnable!=CCX_MODE_ENABLED) && (pParam->paramType!=CCX_ENABLED))
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam when CCX is disabled \n");
        return TI_NOK;
    }

    switch (pParam->paramType)
    {
    case CCX_CONFIGURATION:
        /* CCX configuration includes: RogueAP, CCKM and CKIP */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam, ccxConfiguration=%x \n",pCcxMngr->ccxConfiguration);

        pParam->content.ccxConfiguration = pCcxMngr->ccxConfiguration;
        break; 

    case CCX_CCKM_EXISTS:
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam, ccxCckmExists=%x \n",pCcxMngr->cckmAkmExists);

        /* returns the CCKM existence according to the CCKM AKM existence in the Beacon
         If assoc failed, then return TI_FALSE */
        if (pCcxMngr->useCckmAkm)
        {
            pParam->content.ccxCckmExists = pCcxMngr->cckmAkmExists;
        }
        else
        {
            pParam->content.ccxCckmExists = TI_FALSE;
        }
        break;

#if 0
    case CCX_NEIGHBOR_APS:
        /* Gets the desired list of channel for the scan for Roaming */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam, numberOfAps=%d \n",pCcxMngr->numberOfAps);

        pParam->content.listOfNeighborAps.numOfEntries = pCcxMngr->numberOfAps;
        if (pCcxMngr->numberOfAps==0)
        {
            pParam->content.listOfNeighborAps.pAPListPtr = NULL;
        }
        else
        {
            pParam->content.listOfNeighborAps.pAPListPtr = pCcxMngr->neighborAps;
        }
        break;
#endif
    case CCX_ENABLED:
        /* Enable/Disable CCX features */
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam, CCX_ENABLED=%d \n",pCcxMngr->ccxEnable);
        pParam->content.ccxEnabled = pCcxMngr->ccxEnable;
        break;

    case CCX_CURRENT_AP_SUPPORTED_VERSION:
        /* Get supported CCX version by the current AP (or candidate) */
        TRACE1( pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getParam, supported CCX version=%d\n", pCcxMngr->apCandidateCcxVersion);
        pParam->content.ccxVersion = pCcxMngr->apCandidateCcxVersion;
        break;

    default:
        return TI_NOK;
    }

    return status;
}

/**
 * \brief ccxMngr_sendIAPPPacket - Sends IAPP packets to the tx
 *
 * This function receives an IAPP header and IAPP body packet.
 *      It does the following:
 *      - allocate CtrlBlk and data buffer
 *      - fill the dest and source MAC address
 *      - fill Cisco SNAP
 *      - copy the msg body
 *      - send the CtrlBlk as a 802.11 packet (doesn't go through the convert path of IP packets)
 *      - NOTE: the TX module is responsibe for freeing the packet on TX complete 
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) IAPP_msg_header - the IAPP header \n
 * 3) pPacket - the IAPP body to be sent, excluding the header \n
 * 4) length - the total length of the packet: header + body \n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_sendIAPPPacket(TI_HANDLE hCcxMngr, IAPP_msg_header_t *IAPP_msg_header, TI_UINT8 *pPacket, TI_UINT32 length)
{
    ccxMngr_t       *pCcxMngr = (ccxMngr_t*)hCcxMngr;
    TI_STATUS       status = TI_OK;
    TTxCtrlBlk      *pPktCtrlBlk;
    TI_UINT8        *pPktBuffer;
    paramInfo_t     daParam, saParam, param;
    dot11_header_t  *pDot11Header;
    ScanBssType_e   currBssType;
    TMacAddr        currBssId;
    TI_UINT8        dummyMacAddr[] = DUMMY_MAC_ADDRESS;
    TI_UINT32       headerLength;
    TI_UINT32       pad;
    TI_UINT16       headerFlags;
    TI_BOOL         currentPrivacyInvokedMode;
    TI_UINT8        encryptionFieldSize;

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, " , length= %d\n", length);

    if (pPacket == NULL)
    {
        return TI_OK;
    }

	/* Allocate a TxCtrlBlk and data buffer (large enough for the max IAPP packet) */
    pPktCtrlBlk = TWD_txCtrlBlk_Alloc (pCcxMngr->hTWD);
    pPktBuffer  = txCtrl_AllocPacketBuffer (pCcxMngr->hTxCtrl, pPktCtrlBlk, 2000);
    if (pPktBuffer == NULL)
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, ": No memory\n");
        TWD_txCtrlBlk_Free (pCcxMngr->hTWD, pPktCtrlBlk);
        return TI_NOK;
    }

    /* 
     * If QoS is used, add two bytes padding before the header for 4-bytes alignment.
     * Note that the header length doesn't include it, so the txCtrl detects the pad existence
     *   by checking if the header-length is a multiple of 4. 
     */
    param.paramType = QOS_MNGR_ACTIVE_PROTOCOL;
    qosMngr_getParams(pCcxMngr->hQosMngr, &param);

    if (param.content.qosSiteProtocol == QOS_WME)  
    {
        pad = 2;
        headerLength = WLAN_QOS_HDR_LEN;
        headerFlags = DOT11_FC_DATA_QOS | DOT11_FC_TO_DS;
    }
    else
    {
        pad = 0;
        headerLength = WLAN_HDR_LEN;
        headerFlags = DOT11_FC_DATA | DOT11_FC_TO_DS;
    }

    /* 
     * Handle encryption if needed (decision was done at RSN and is provided by TxCtrl):
     *   - Set WEP bit in header.
     *   - Add padding for FW security overhead: 4 bytes for TKIP, 8 for AES.  
     */
    txCtrlParams_getCurrentEncryptionInfo (pCcxMngr->hTxCtrl, 
                                           &currentPrivacyInvokedMode,
                                           &encryptionFieldSize);
    if (currentPrivacyInvokedMode)
    {
        headerFlags |= DOT11_FC_WEP;
        headerLength += encryptionFieldSize;
    }
    
    pDot11Header = (dot11_header_t*)(pPktCtrlBlk->aPktHdr + pad);

    COPY_WLAN_WORD(&pDot11Header->fc, &headerFlags); /* copy with endianess handling. */

    /* Get the Destination MAC address */
    daParam.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
    status = ctrlData_getParam(pCcxMngr->hCtrlData, &daParam);
    if (status != TI_OK)
    {
        txCtrl_FreePacket (pCcxMngr->hTxCtrl, pPktCtrlBlk, TI_NOK);
        return TI_NOK;
    }

    /* Get the Source MAC address */
    saParam.paramType = CTRL_DATA_MAC_ADDRESS;
    status = ctrlData_getParam(pCcxMngr->hCtrlData, &saParam);
    if (status != TI_OK)
    {
        txCtrl_FreePacket (pCcxMngr->hTxCtrl, pPktCtrlBlk, TI_NOK);
        return TI_NOK;
    }

    /* receive BssId and Bss Type from control module */
    ctrlData_getCurrBssTypeAndCurrBssId(pCcxMngr->hCtrlData, &currBssId, &currBssType);
    if (currBssType != BSS_INFRASTRUCTURE)
    {
        txCtrl_FreePacket (pCcxMngr->hTxCtrl, pPktCtrlBlk, TI_NOK);
        return TI_NOK;
    }

    /* copy BSSID */
    MAC_COPY (pDot11Header->address1, currBssId);
    /* copy source mac address */
    MAC_COPY (pDot11Header->address2, saParam.content.ctrlDataCurrentBSSID);
    /* copy destination mac address*/
    MAC_COPY (pDot11Header->address3, daParam.content.ctrlDataCurrentBSSID);
    
    /* If not preset, copy destination mac address */
    if (MAC_EQUAL (dummyMacAddr, IAPP_msg_header->destinationMacAddress))
    {
        MAC_COPY (IAPP_msg_header->destinationMacAddress, daParam.content.ctrlDataCurrentBSSID);
    }

    /* copy source mac address */
    MAC_COPY (IAPP_msg_header->sourceMacAddress, saParam.content.ctrlDataCurrentBSSID);

    /* Copy the IAPP header */
    os_memoryCopy (pCcxMngr->hOs, pPktBuffer, (TI_UINT8 *)IAPP_msg_header, IAPP_PACKET_HEADER_LEN);

    /* Copy the rest of the data */
    os_memoryCopy(pCcxMngr->hOs, pPktBuffer + IAPP_PACKET_HEADER_LEN, pPacket, length - IAPP_PACKET_HEADER_LEN);

    /* Update packet parameters (start-time, length, pkt-type) */
    pPktCtrlBlk->tTxDescriptor.startTime = os_timeStampMs (pCcxMngr->hOs);
    pPktCtrlBlk->tTxPktParams.uPktType   = TX_PKT_TYPE_WLAN_DATA;
    BUILD_TX_TWO_BUF_PKT_BDL (pPktCtrlBlk, (TI_UINT8 *)pDot11Header, headerLength, pPktBuffer, length)

    /* Mark the packet in order to save the number of retries for the next link test packet*/
    if (IAPP_msg_header->messageType == IAPP_LINK_TEST)
    {
        pPktCtrlBlk->tTxPktParams.uFlags |=  TX_CTRL_FLAG_LINK_TEST;
    }

    /* Enqueue packet in the mgmt-queues and run the scheduler. */
    status = txMgmtQ_Xmit (pCcxMngr->hTxMgmtQ, pPktCtrlBlk, TI_FALSE);

    return status;
}

/**
 * \brief ccxMngr_parseCcxVer - Parses the CCX version Number IE sent in the Beacon/Probe Response
 *
 * This function receives the variable IE buffer (from the Beacon or Probe Response).
 *      - search for CCX Version Number IE
 *      - parse and check validity
 *      - set the apCandidateCcxVersion accordingly (supported version if was found, 0 otherwise)
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) pIeBuffer - IE section of the Beacon/Probe Response \n
 * 3) length - the total length of the packet \n
 * Return Value: CCX version number; in case of non CCX AP, returns 0\n
 */
TI_UINT8 ccxMngr_parseCcxVer(TI_HANDLE hCcxMngr, TI_UINT8 *pIeBuffer, TI_UINT16 length)
{
    ccxMngr_t                   *pCcxMngr;
    ccxVersionNumIePacket_t     *pCcxIe;
    TI_UINT8                       ccxOuiBuffer[DOT11_OUI_LEN+1]=CCX_OUI;


    if ((hCcxMngr==NULL) || (pIeBuffer==NULL))
    {
        return 0;
    }

    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    pCcxMngr->apCandidateCcxVersion = 0;

    ccxOuiBuffer[DOT11_OUI_LEN] = CCX_VERSION_NUMBER_OUI_TYPE;
    /* search in the buffer pointer to the beginning of the
        CCX Ver Num IE according to the IE ID, OUI and type */
    if (!mlmeParser_ParseIeBuffer (pCcxMngr->hOs, pIeBuffer, length, WPA_IE_ID, (TI_UINT8**)(&pCcxIe), ccxOuiBuffer, DOT11_OUI_LEN+1))
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_parseCcxVer, no CCX Ver was found \n");
        return 0;
    }
    /* store in apCandidateCcxVersion the version if found */
    if (pCcxIe->length == CCX_VERSION_NUMBER_LEN)
    {
        pCcxMngr->apCandidateCcxVersion = pCcxIe->VerNum;
        TRACE1( pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_parseCcxVer, supported version: %d\n",pCcxIe->VerNum);
    }
    return (pCcxMngr->apCandidateCcxVersion);
}

/**
 * \brief ccxMngr_startCckm - Called upon Roaming event to start CCKM - fast handoff
 *
 * This function is called when Roaming trigger occur, and a new AP was found,
 *      before connection to the new AP.
 *      - check if CCKM exits, 
 *      - if not return Roaming reply
 *      - if CCKM exist, invoke a CCKM_START event to the supplicant
 *          with the new AP MAC and TSF, and start a timer for getting the reply 
 *          from the supplicant
 *  
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) macAddress - MAC adrress of the AP to which roaming will be initiated \n
 * 3) tsfTimeStamp - TSF TS of the AP to which roaming will be initiated \n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_startCckm (TI_HANDLE hCcxMngr, TMacAddr *macAddress, TI_UINT8 *tsfTimeStamp)
{
    
    ccxMngr_t               *pCcxMngr;
    OS_CCX_CCKM_START       cckmStartBuffer;

    if ((hCcxMngr==NULL) || (macAddress==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    TRACE14(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_startCckm, macAddress=%x-%x-%x-%x-%x-%x, tsfTimeStamp=%x%x%x%x%x%x%x%x \n", (*macAddress)[0], (*macAddress)[1], (*macAddress)[2], (*macAddress)[3], (*macAddress)[4], (*macAddress)[5], tsfTimeStamp[0], tsfTimeStamp[1], tsfTimeStamp[2], tsfTimeStamp[3], tsfTimeStamp[4], tsfTimeStamp[5], tsfTimeStamp[6], tsfTimeStamp[7]);
    if (pCcxMngr->cckmAkmExists)
    {
        os_memoryCopy(pCcxMngr->hOs, (void*)cckmStartBuffer.Timestamp, tsfTimeStamp, TIME_STAMP_LEN); 
        os_memoryCopy(pCcxMngr->hOs, (void*)cckmStartBuffer.BSSID,macAddress, 6); 

        tmr_StartTimer (pCcxMngr->hCckmTimer,
                        ccxMngr_cckmTimerExpired,
                        (TI_HANDLE)pCcxMngr,
                        CCX_CCKM_DELAY_MS,
                        TI_FALSE);

        EvHandlerSendEvent(pCcxMngr->hEvHandler, IPC_EVENT_CCKM_START, (TI_UINT8*)&cckmStartBuffer,sizeof(OS_CCX_CCKM_START));

        return TI_NOK;
    }
    else
    {   /* indicate that Roaming reply was received,
            without CCKM */
        pCcxMngr->useCckmFastRoaming = TI_FALSE;
        apConn_RoamHandoffFinished(pCcxMngr->hAPConn);
        return TI_OK;
    }
}


/**
 * \brief ccxMngr_getCckmInfoElement - Called in Association Request build in order to get the CCKM IE
 * if applicable.
 *
 *  This function is called in ASSOC REQ in order to build the CCKM 
 *      IE if it exists.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) pCckmIe - the CCKM IE, or NULL if isn't required \n
 * 3) pCckmIeLen - the CCKM IE length, or 0 if isn't required \n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_getCckmInfoElement(TI_HANDLE hCcxMngr, TI_UINT8 *pCckmIe, TI_UINT8 *pCckmIeLen)
{
    ccxMngr_t           *pCcxMngr;
    
    if ((hCcxMngr==NULL) || (pCckmIe==NULL) || (pCckmIeLen==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;
    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getCckmInfoElement, useCckm=%d \n", pCcxMngr->useCckmFastRoaming);
    
    if (pCcxMngr->useCckmFastRoaming)
    {
        os_memoryCopy(pCcxMngr->hOs, pCckmIe, pCcxMngr->associationRequestIE, pCcxMngr->associationRequestIELength);
        *pCckmIeLen = pCcxMngr->associationRequestIELength;
    }
    else
    {
        *pCckmIeLen = 0;
    }
    return TI_OK;

}

/**
 * \brief ccxMngr_getCCXVersionInfoElement - Called in Association Request build in order to get the 
 * CCX version number IE if applicable.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) pCcxVerIe - the CCX version no IE, or NULL if isn't required \n
 * 3) pCckmIeLen - the CCX version no IE length, or 0 if isn't required \n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_getCCXVersionInfoElement(TI_HANDLE hCcxMngr, TI_UINT8 *pCcxVerIe, TI_UINT8 *pCcxVerIeLen)
{
    ccxMngr_t           *pCcxMngr;
    
    if ((hCcxMngr==NULL) || (pCcxVerIe==NULL) || (pCcxVerIeLen==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;
    
    if (pCcxMngr->apCandidateCcxVersion >= 2)
    {
        ccxVersionNumIePacket_t ccxVerNumIePacket = {WPA_IE_ID, CCX_VERSION_NUMBER_LEN, CCX_OUI, CCX_VERSION_NUMBER_OUI_TYPE, 0};

        ccxVerNumIePacket.VerNum = pCcxMngr->ccxSupportedVersion;
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getCCXVersionInfoElement: CCX_VER_NUM_IE exists\n");

        *pCcxVerIeLen = CCX_VERSION_NUMBER_LEN+2;
        os_memoryCopy(pCcxMngr->hOs, pCcxVerIe, &ccxVerNumIePacket, *pCcxVerIeLen);
    }
    else
    {
        *pCcxVerIeLen = 0;
    }
    return TI_OK;

}

/**
 * \brief ccxMngr_isIappPacket - Called by RX when a data packet is received and checks if the
 *  packet is an IAPP packet.
 *      This function decides if a packet is IAPP according to the IAPP header,
 *  which includes: Aironet SNAP, IAPP length, IAPP type, IAPP function type
 *  dest and source MAC addresses.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) pWbuf - the received WBUF \n
 * Return Value: TI_TRUE if the packet is IAPP packet, TI_FALSE otherwise.\n
 */
TI_BOOL ccxMngr_isIappPacket(TI_HANDLE hCcxMngr, void *pBuffer)
{
    TI_UINT8            *pIappPacket;
    IAPP_msg_header_t   *pIappHeader;
    ccxMngr_t           *pCcxMngr;
    TI_UINT8            iappSnap[8] = IAPP_PACKET_SNAP;
	TI_UINT8            *dataBuf;
    TI_UINT32           headerLength;

    if ((hCcxMngr==NULL) || (pBuffer==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;
    dataBuf  = (TI_UINT8 *)RX_BUF_DATA(pBuffer);
    GET_MAX_HEADER_SIZE (dataBuf, &headerLength);

    pIappPacket = dataBuf + headerLength;

    /* parse the IAPP header */
    pIappHeader = (IAPP_msg_header_t*)pIappPacket;

    /* Check if the packet has Cisco SNAP */
    if (os_memoryCompare(pCcxMngr->hOs, (void*)pIappHeader->snapHeader, iappSnap, WLAN_SNAP_HDR_LEN))
    {
        TRACE8(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_parseIAPPPacket , not Cisco snapHeader=%x %x %x %x %x %x %x %x\n", pIappHeader->snapHeader[0], pIappHeader->snapHeader[1], pIappHeader->snapHeader[2], pIappHeader->snapHeader[3], pIappHeader->snapHeader[4], pIappHeader->snapHeader[5], pIappHeader->snapHeader[6], pIappHeader->snapHeader[7]);
        return TI_FALSE;

    }
    return TI_TRUE;

}

/**
 * \brief ccxMngr_recvIAPPPacket - Called by RX when an IAPP data packet is received, and distributes it
 * to the requesting objects.
 *      Modules that were registered to recv IAPP packets using: ccxMngr_registerForRecvIappPacket
 *  are called to recv the IAPP type that they were registered for.
 *  This function is not responsible to FREE the WBUF. This is done by the calling RX.c module.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) pBuffer - the received Buffer. \n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_recvIAPPPacket (TI_HANDLE hCcxMngr, void *pBuffer, TRxAttr *pRxAttr)
{
    TI_UINT8            *pIappPacket;
    IAPP_msg_header_t   *pIappHeader;
    ccxMngr_t           *pCcxMngr;
    TI_UINT8            iappSnap[8] = IAPP_PACKET_SNAP;
    iappType_e          iappType;
    TI_UINT8            iappObjectCounter, currentIappObjectCounter;
	TI_UINT8            *dataBuf;
    TI_UINT32           headerLength;
    
    if ((hCcxMngr==NULL) || (pBuffer==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    if (pCcxMngr->ccxEnable!=CCX_MODE_ENABLED)
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_recvIAPPPacket, CCX is disabled \n");
        return TI_OK;
    }
    /* Get the start of the IAPP packet */
    dataBuf  = (TI_UINT8 *)RX_BUF_DATA(pBuffer);
    GET_MAX_HEADER_SIZE (dataBuf, &headerLength);

    pIappPacket =  dataBuf + headerLength;

    /* parse the IAPP header */
    pIappHeader = (IAPP_msg_header_t*)pIappPacket;
    
    /* If the packet doesn't have Cisco SNAP, drop the packet */
    if (os_memoryCompare(pCcxMngr->hOs, (void*)pIappHeader->snapHeader, iappSnap, 8))
    {
        TRACE8(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_parseIAPPPacket , not Cisco snapHeader=%x %x %x %x %x %x %x %x\n", pIappHeader->snapHeader[0], pIappHeader->snapHeader[1], pIappHeader->snapHeader[2], pIappHeader->snapHeader[3], pIappHeader->snapHeader[4], pIappHeader->snapHeader[5], pIappHeader->snapHeader[6], pIappHeader->snapHeader[7]);
        return TI_NOK;

    }
    iappType = (iappType_e)pIappHeader->messageType;
    /* If the packet doesn't have a recognized IAPP type, drop the packet */
    if ((iappType<IAPP_FIRST_TYPE) || (iappType>=IAPP_LAST_TYPE))
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_parseIAPPPacket , unsupported IAPP type=%x\n",iappType);
        return TI_NOK;
    }
    
    /* look for handlers that were registered for the IAPP packet type, and call them */
    currentIappObjectCounter = pCcxMngr->iappPacketRecvRegisterObjectsCounter[iappType-IAPP_FIRST_TYPE];
    for (iappObjectCounter=0; iappObjectCounter<currentIappObjectCounter; iappObjectCounter++)
    {
        iappParsingRegistrationTable_t  *pIappObject = &pCcxMngr->iappPacketRecvRegisterObjectsTable[iappType-IAPP_FIRST_TYPE][iappObjectCounter];

        if (pIappObject->iappParsingRegistrationProcedure != NULL)
        {
            /* in order to use RSSI value */
            if(iappType == IAPP_LINK_TEST)
                pCcxMngr->pRxAttr = pRxAttr;

            pIappObject->iappParsingRegistrationProcedure(pIappObject->handler, pIappPacket);
        }
    }
    
    return TI_OK;
    
}

/**
 * \brief ccxMngr_registerForRecvIappPacket - Called by any object that requests to register for receiving 
 * an IAPP packet.
 *      The registartion is done for a specific IAPP type. The requesting module supplies
 *  handle and procedure to be called.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) iappParsingRegistration - a handle and procedure to be called when the IAPP packet is recieved\n
 * 3) iappType - the requested IAPP packet type\n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_registerForRecvIappPacket(TI_HANDLE hCcxMngr, iappParsingRegistrationTable_t iappParsingRegistration, iappType_e iappType)
{
    ccxMngr_t           *pCcxMngr;
    TI_UINT8               iappCounter;
    
    if (hCcxMngr==NULL)
    {   
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;
    
    if ((iappParsingRegistration.handler==NULL) || (iappParsingRegistration.iappParsingRegistrationProcedure==NULL))
    {   /* bad input parameter */
        TRACE2(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_registerForRecvIappPacket , bad input parameters: handler=%p, proc=%p\n", iappParsingRegistration.handler, iappParsingRegistration.iappParsingRegistrationProcedure);
        return TI_NOK;
    }

    if ((iappType<IAPP_FIRST_TYPE) || (iappType>=IAPP_LAST_TYPE))
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_registerForRecvIappPacket , iappType=%d\n",iappType);
        return TI_NOK;

    }
    iappType = (iappType_e)(iappType-IAPP_FIRST_TYPE);
    iappCounter = pCcxMngr->iappPacketRecvRegisterObjectsCounter[iappType];
    if (iappCounter >= MAX_OBJECTS_REGISTERS_FOR_IAPP_PACKET)
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_registerForRecvIappPacket , iappPacketRecvRegisterObjectsCounter=%d\n",iappCounter);
        return TI_NOK;
    }
    /* register the requesting module for the desired IAPP type */
    pCcxMngr->iappPacketRecvRegisterObjectsTable[iappType][iappCounter].handler = iappParsingRegistration.handler;
    pCcxMngr->iappPacketRecvRegisterObjectsTable[iappType][iappCounter].iappParsingRegistrationProcedure = iappParsingRegistration.iappParsingRegistrationProcedure;
    pCcxMngr->iappPacketRecvRegisterObjectsCounter[iappType] += 1;

    TRACE3(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_registerForRecvIappPacket - success, Counter=%d, handle = %p, proc=%p\n", pCcxMngr->iappPacketRecvRegisterObjectsCounter[iappType], pCcxMngr->iappPacketRecvRegisterObjectsTable[iappType][iappCounter].handler, pCcxMngr->iappPacketRecvRegisterObjectsTable[iappType][iappCounter].iappParsingRegistrationProcedure);

    
    return TI_OK;

}

/**
 * \brief ccxMngr_updateIappInformation - Called by the SME when entering or exiting a CONNECTED state. 
 *      - When entering a CONNECTED state, an IAPP information packet is sent.
 *      - When exiting a CONNECTED state, the disassociation time is set to zero.
 *      - When entering a CONNECTED state and the reason for the previous disassoc
 *          was Rogue AP, no IAPP packet is sent.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) assocStatus - TI_TRUE if CONNECTED, TI_FALSE otherwise.\n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_updateIappInformation(TI_HANDLE hCcxMngr, assocStatus_e assocStatus)
{
    ccxMngr_t                       *pCcxMngr;
    IAPP_Report_Information_msg_t   IAPP_Report_Information_msg =
                                        {{IAPP_PACKET_SNAP, 0, IAPP_REPORT_INFORMATION, 
                                        IAPP_FUNCTION_TYPE, DUMMY_MAC_ADDRESS, DUMMY_MAC_ADDRESS}, 
                                        {{0,0, IAPP_CISCO_AIRONET_OUI, DUMMY_MAC_ADDRESS, 0, 0, "", 0}}};
    static TI_UINT8 apIpOuiIe[4] = { 0x00, 0x40, 0x96, 0x00};
	apConn_roamingTrigger_e			assocRoamingTrigger = ROAMING_TRIGGER_NONE;
	TI_UINT8						assocReasonCode = ASSOC_REASON_UNSPECIFIED;

    TI_STATUS                       status;
    TI_UINT32                          disassociateTime;
    TI_BOOL                            comparePrev2CurAP;
    paramInfo_t                     param;


    IAPP_Report_Information_msg.IAPP_Report_Information_header.length=HTOWLANS(IAPP_REPORT_INFORMATION_LENGTH);
    IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].tag_adjacentApReport = HTOWLANS(TAG_ADJACENT_AP_REPORT);
    IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].length = HTOWLANS(IAPP_REPORT_INFORMATION_BODY_LENGTH);

    if (hCcxMngr==NULL)
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;
    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , assocStatus= %d\n",assocStatus);
 
    switch (assocStatus)
    {
    case CCX_DISASSOC:
    {   /* Disassociation */
        /* Reset the last List Of Channels */
        pCcxMngr->neighborAPList.numOfEntries = 0;
        os_memoryZero(pCcxMngr->hOs, pCcxMngr->neighborAPList.APListPtr, sizeof(neighborAP_t)*MAX_NUM_OF_NEIGHBOR_APS);

        if (pCcxMngr->disAssociateTime == 0)
        {   /* This is the first DisAssociation, after Association */
            pCcxMngr->disAssociateTime = os_timeStampMs(pCcxMngr->hOs);

            TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , disAssociateTime - start= 0x%x\n",pCcxMngr->disAssociateTime);
            pCcxMngr->sendIappInfo = TI_TRUE;
        }
        else
        {   /* second Disassociation without successful assoc, do we need to send IAPP packet ???*/
            /*pCcxMngr->sendIappInfo = TI_FALSE;*/
        }
        pCcxMngr->associated = TI_FALSE;
        break;
    }
    case CCX_ROGUE:
    {   /* Roague AP - erase from prev AP param and do not send IAPP packet */
        pCcxMngr->neighborAPList.numOfEntries = 0;
        os_memoryZero(pCcxMngr->hOs, pCcxMngr->neighborAPList.APListPtr, sizeof(neighborAP_t)*MAX_NUM_OF_NEIGHBOR_APS);
        os_memoryZero(pCcxMngr->hOs, (void*)pCcxMngr->prevApParams.prevAssocBssid, MAC_ADDR_LEN);
        os_memoryZero(pCcxMngr->hOs, (void*)pCcxMngr->prevApParams.prevAssocSsid.str, MAX_SSID_LEN);
        pCcxMngr->prevApParams.prevAssocSsid.len=0;
        pCcxMngr->prevApParams.prevAssocChannel=0;

        pCcxMngr->sendIappInfo = TI_FALSE;
        pCcxMngr->associated = TI_FALSE;
        break;
    }
    case CCX_ASSOC_OK:
        pCcxMngr->associated = TI_TRUE;
        pCcxMngr->cckmRetriesCounter = 0;
        pCcxMngr->useCckmAkm = TI_TRUE;
        /* Send Roage AP if required */

        if (pCcxMngr->ccxConfiguration & OS_CCX_CONFIGURATION_ENABLE_ROGUE_AP)
        {
            param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
            ctrlData_getParam(pCcxMngr->hCtrlData, &param);
            status = rogueAp_report(pCcxMngr->hRogueAp, (TI_UINT8 *)param.content.ctrlDataCurrentBSSID);
        }

        break;
    default: 
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_updateIappInformation , assocStatus = %d \n", assocStatus);
        return TI_NOK;
/*        break; */
    }

    /* When to send the IAPP packet, according to Aironet, CCX V2, other?
       According to Cisco's reply on this issue, always send the IAPP packet.*/
#if 0
    /* This should be the correct way - need to convince Cisco */
    if (!pCcxMngr->apCandidateCcxVersion < 2)
    {   
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , No ccxV2IeExists, do not send IAPP packet\n");
        return TI_OK;
    }
#endif
    TRACE3(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , disAssociateTime - end= 0x%x, delta=%d, \n cckmRetriesCounter= %d\n",os_timeStampMs(pCcxMngr->hOs), os_timeStampMs(pCcxMngr->hOs)-pCcxMngr->disAssociateTime, pCcxMngr->cckmRetriesCounter);

    if (assocStatus!=CCX_ASSOC_OK)
    {   /* In disassoc not due to Roague AP, need to send IAPP packet on the next assoc */
        if (pCcxMngr->cckmAkmExists)
        {   /* If CCKM is used increase a counter of failures */
            pCcxMngr->cckmRetriesCounter++;
            TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , cckmRetriesCounter= %d\n",pCcxMngr->cckmRetriesCounter);

        }
        if (pCcxMngr->cckmRetriesCounter > CCKM_REUATH_RETRIES_LIMIT)
        {   /* If CCKM was used and the number of failures was too high,
                try the next connection without CCKM -
                regular WPA */
            param.paramType = WPA_801_1X_AKM_EXISTS;
            rsn_getParam(pCcxMngr->hRsn, &param);

            TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "do/not use CCKM anymore, wpa_802_1x_AkmExists=%d \n", param.content.wpa_802_1x_AkmExists);
            if (param.content.wpa_802_1x_AkmExists)
            {   /* Use 802_1x AKM, instaed of CCKM */
                pCcxMngr->useCckmAkm = TI_FALSE;
                pCcxMngr->useCckmFastRoaming = TI_FALSE;
                pCcxMngr->cckmRetriesCounter = 0;
            }
        }
        return TI_OK;
    }
    /* assoc succesful - check if required to send IAPP packet */
    disassociateTime = pCcxMngr->disAssociateTime;
    pCcxMngr->disAssociateTime = 0;

    param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
    ctrlData_getParam(pCcxMngr->hCtrlData, &param);

    comparePrev2CurAP = !MAC_EQUAL (param.content.ctrlDataCurrentBSSID, 
                                    pCcxMngr->prevApParams.prevAssocBssid);
    /* send Report Information IAPP packet to the AP */
    if ((pCcxMngr->ccxEnable != CCX_MODE_ENABLED) || (!pCcxMngr->sendIappInfo) || (!comparePrev2CurAP))
    {   /* Do not send IAPP packet if CCX is disabled or previous failure was due to
            Rogue AP, or the associated AP is the same as the previous one */
        TRACE3(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation , Do not send IAPP packet. ccxEnable=%d, sendIappInfo=%d, comparePrev2CurAP=%d \n", pCcxMngr->ccxEnable, pCcxMngr->sendIappInfo, comparePrev2CurAP);
        /* update prev assoc params*/       
        ccxMngr_updatePrevAssocParams(pCcxMngr);
        return TI_OK;
    }

    /* succesful Re-Association - send IAPP Information Report packet */
    pCcxMngr->sendIappInfo = TI_FALSE;
    
    /* copy the header */
    
    /* generate the body */
    /* get the previous AP mac */
    MAC_COPY (IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac, 
              pCcxMngr->prevApParams.prevAssocBssid);
    /* get the previous AP ssid */
    IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].ssidLength = HTOWLANS(pCcxMngr->prevApParams.prevAssocSsid.len);
    os_memoryCopy(pCcxMngr->hOs, (TI_UINT8*)IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].ssid, (TI_UINT8*)pCcxMngr->prevApParams.prevAssocSsid.str, pCcxMngr->prevApParams.prevAssocSsid.len);
    /* get the previous AP channel */
    IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].channel = HTOWLANS(pCcxMngr->prevApParams.prevAssocChannel);

    /* update the disconnect time */
    disassociateTime = (os_timeStampMs(pCcxMngr->hOs) - disassociateTime)/1000; /* time in seconds */
    IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].disassociateTime = HTOWLANS(disassociateTime);

    /* update the assoc roam information */
    IAPP_Report_Information_msg.IAPP_AssocRoam_msg_body.tag_assocRoam = HTOWLANS(TAG_ASSOC_ROAM_REPORT);
    IAPP_Report_Information_msg.IAPP_AssocRoam_msg_body.length = HTOWLANS(IAPP_ASSOC_ROAM_LENGTH);
    os_memoryCopy(pCcxMngr->hOs, &(IAPP_Report_Information_msg.IAPP_AssocRoam_msg_body.aironetOui), &apIpOuiIe, sizeof(apIpOuiIe));
    
	apConn_getAssocRoamingTrigger(pCcxMngr->hAPConn, &assocRoamingTrigger);
	
	if (assocRoamingTrigger == ROAMING_TRIGGER_NONE)	
	{
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_updateIappInformation, assocRoamingTrigger == ROAMING_TRIGGER_NONE ! \n");
		return TI_NOK;
	}

	assocReasonCode = ccxMngr_translateAssocRoamingTriggerToReasonCode(pCcxMngr, assocRoamingTrigger);

	IAPP_Report_Information_msg.IAPP_AssocRoam_msg_body.associationReasonCode = assocReasonCode;

    TRACE8(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "Send IAPP Information packet, prev MAC= 0x%x-0x%x-0x%x-0x%x-0x%x-0x%x, channel=%d\n, ssid=, disAssocTime=%d\n", IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[0], IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[1], IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[2], IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[3], IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[4], IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].previousApMac[5], HTOWLANS(IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].channel), HTOWLANS(IAPP_Report_Information_msg.IAPP_Report_Information_msg_body[0].disassociateTime));

    /* update prev assoc params*/
    ccxMngr_updatePrevAssocParams(pCcxMngr);

    /* send the IAPP packets */
    status = ccxMngr_sendIAPPPacket(hCcxMngr, &IAPP_Report_Information_msg.IAPP_Report_Information_header, (TI_UINT8*)&IAPP_Report_Information_msg.IAPP_Report_Information_msg_body, IAPP_REPORT_INFORMATION_LENGTH);
    
    pCcxMngr->IappSentInfoTS=os_timeStampMs(pCcxMngr->hOs);

    return status;
}

/**
 * \brief ccxMngr_rogueApDetected - Called by RSN when authnetication or association in LEAP was un-successful. 
 *  When a Rogue AP was detected, it is saved in the Rogue AP module DB.
 *      Then when successful LEAP authentication is established, Rogue AP ,
 *      is sent per each Rogue AP that was found, and the DB is reset.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) failureReason - the roague AP failure reason: invalid type, timeout, challenge to/from AP failed.\n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_rogueApDetected(TI_HANDLE hCcxMngr, EAuthStatus failureReason)
{

    OS_CCX_ROGUE_AP_DETECTED    rogueApDetected;
    ccxMngr_t                   *pCcxMngr;
    paramInfo_t                 param;
    TI_STATUS                   status = TI_OK;
    
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_rogueApDetected, failureReason=%d \n", failureReason);

    if (pCcxMngr->associated)
    {   /* Ignore Rogue AP after AP is already associated */
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_rogueApDetected, Already associated! \n");

        return TI_OK;
    }
    param.paramType = RSN_CCX_NETWORK_EAP;

    status = rsn_getParam(pCcxMngr->hRsn, &param);
    
    if (status != TI_OK)
    {       
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_rogueApDetected, rsn_getParam() failed \n");
        return status;
    }

    
    if ((pCcxMngr->ccxConfiguration & OS_CCX_CONFIGURATION_ENABLE_ROGUE_AP) &&
        (param.content.networkEap != OS_CCX_NETWORK_EAP_OFF))
    {

        /* Only in LEAP authentication Rogue is indicated */
        rogueApDetected.FailureReason = failureReason;
        
        param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
        status = ctrlData_getParam(pCcxMngr->hCtrlData, &param);
        MAC_COPY (rogueApDetected.RogueAPMacAddress, param.content.siteMgrDesiredBSSID);
        
        /* Since the AP name is not known, it should be zero */
        os_memoryZero(pCcxMngr->hOs, (TI_UINT8*)rogueApDetected.RogueAPName, 16);

        param.paramType = CCX_ROGUE_AP_DETECTED;
        os_memoryCopy(pCcxMngr->hOs, &param.content.rogueApDetected, &rogueApDetected, sizeof(OS_CCX_ROGUE_AP_DETECTED)); 
        status = ccxMngr_setParam(pCcxMngr, &param);
    }
    return status;

}

/**
 * \brief ccxMngr_registerForCcxEnableStatus - Called by any object that requests to query the CCX 
 * enable/disable status. 
 *      The requesting module supplies handle and procedure to be called.
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) ccxEnableStatusRegistration - a proceure and handle to be called.\n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_registerForCcxEnableStatus(TI_HANDLE hCcxMngr, ccxEnableStatusRegistrationTable_t ccxEnableStatusRegistration)
{
    ccxMngr_t           *pCcxMngr;
    TI_UINT8               ccxEnableCounter;
    
    if ((hCcxMngr==NULL) || (ccxEnableStatusRegistration.handler==NULL) || ccxEnableStatusRegistration.ccxEnableStatusRegistrationProcedure==NULL)
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    ccxEnableCounter = pCcxMngr->ccxEnableStatusRegistrationCounter;
    if (ccxEnableCounter >= MAX_OBJECTS_REGISTERS_FOR_CCX_ENABLE)
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_registerForCcxEnableStatus , ccxEnableStatusRegistrationCounter=%d\n",ccxEnableCounter);
        return TI_NOK;
    }
    
    pCcxMngr->ccxEnableStatusRegistrationTable[ccxEnableCounter].handler = ccxEnableStatusRegistration.handler;
    pCcxMngr->ccxEnableStatusRegistrationTable[ccxEnableCounter].ccxEnableStatusRegistrationProcedure = ccxEnableStatusRegistration.ccxEnableStatusRegistrationProcedure;
    pCcxMngr->ccxEnableStatusRegistrationCounter++;
    
    return TI_OK;


}

/**
 * \brief ccxMngr_getCurMAcAddress - Called by admCtrl to get the current MAC address. 
 * 
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) macAddress - the current MAC address.\n
 * Return Value: TI_OK or TI_NOK.\n
 */
TI_STATUS ccxMngr_getCurMAcAddress(TI_HANDLE hCcxMngr, TMacAddr *macAddress)
{
    ccxMngr_t           *pCcxMngr;
    paramInfo_t         paramInfo;
    
    if (hCcxMngr==NULL)
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    paramInfo.paramType = CTRL_DATA_MAC_ADDRESS;
    ctrlData_getParam(pCcxMngr->hCtrlData, &paramInfo);
    MAC_COPY (*macAddress, paramInfo.content.ctrlDataDeviceMacAddress); 
    return TI_OK;

}

/************************************************************************
 *                        ccxMngr_buildCcxTS_IEs                        *
 ************************************************************************
DESCRIPTION: The function checks if the current AP is CCX version 4 or above
             if so, it builds TSRS info-element
                                                                                                   
INPUT:      CCX Manager handle.
             
OUTPUT:     

RETURN:     -

************************************************************************/
void ccxMngr_buildCcxTS_IEs(TI_HANDLE hCcxMngr, TI_UINT8 *pDataBuff, TI_UINT32 *len, TI_UINT8 tid)
{
    ccxMngr_t           *pCcxMngr;

    pCcxMngr = (ccxMngr_t *)hCcxMngr;

    /* In case of CCX version 4 or above, send TSRS IE */
    if (pCcxMngr->apCandidateCcxVersion >= 4) 
    {
        ccxMngr_buildTsrsIE(tid, pDataBuff, len);
    }
    else
        *len = 0;
}


/************************************************************************
 *                        ccxMngr_setCCXQoSParams                       *
 ************************************************************************
DESCRIPTION: The function updates different modules in the system with 
             the parameters received in CCX IEs
                                                                                                   
INPUT:      CCX Manager handle.
            ies      -  pointer to received IE parameters. 
OUTPUT:     

RETURN:     -

************************************************************************/
void ccxMngr_setCCXQoSParams(TI_HANDLE hCcxMngr, CCXv4IEs_t *ies, TI_UINT8 ac)
{
    paramInfo_t param;
    ccxMngr_t           *pCcxMngr;

    pCcxMngr = (ccxMngr_t *)hCcxMngr;

    /* Check if TSRS IE is present */
    if (ies->trafficStreamParameter != NULL)
    {
        param.content.txDataQosParams.acID = (TI_UINT8)WMEQosTagToACTable[ies->trafficStreamParameter->tsid];
        param.content.txDataQosParams.tsrsArr = (TI_UINT8 *)ies->trafficStreamParameter->tsRates;
        param.content.txDataQosParams.tsrsArrLen = ies->trafficStreamParameter->hdr[1] - 5;
        param.paramType = CTRL_DATA_TSRS_PARAM;
        ctrlData_setParam(pCcxMngr->hCtrlData, &param);

        /* Update FW by re-setting Control enable parameter */
        param.paramType = CTRL_DATA_RATE_CONTROL_ENABLE_PARAM;
        ctrlData_getParam(pCcxMngr->hCtrlData, &param);

        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setCCXQoSParams: TSRS IE received, ac=%d\n", param.content.txDataQosParams.acID);
        ctrlData_setParam(pCcxMngr->hCtrlData, &param);
    }

    /* Check if MSDU lifetime IE is present */
    if (ies->edcaLifetimeParameter != NULL)
    {
        TI_UINT16 msduLifeTimeTu; 
        TI_UINT32 msduLifeTimeMsec;
        TI_UINT8 *pFrameMsduLifeTime = ((TI_UINT8 *)(ies->edcaLifetimeParameter)) + 7;
        /* Note: The dot11_MSDU_LIFE_TIME_IE_t structure is unpacked so the msduLifeTime
                   field is accessed directly to its offset in the frame structure. */ 

        COPY_WLAN_WORD(&msduLifeTimeTu ,pFrameMsduLifeTime);
        msduLifeTimeMsec = TUs_TO_MSECs((TI_UINT32)msduLifeTimeTu);

        /* configure MSDU-Lifetime to TxCtrl. */
        txCtrlParams_setAcMsduLifeTime(pCcxMngr->hTxCtrl, 
                                       WMEQosTagToACTable[ies->edcaLifetimeParameter->tsid], 
                                       msduLifeTimeMsec);

        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_setCCXQoSParams: MSDU Lifetime IE received = %d msec\n", msduLifeTimeMsec);
    }

    /* Check if TS metrix IE is present */
    if (ies->tsMetrixParameter == NULL)
    {
        measurementMgr_disableTsMetrics(pCcxMngr->hMeasurementMngr, ac);
    }
    else
    {
        measurementMgr_enableTsMetrics (pCcxMngr->hMeasurementMngr, ies->tsMetrixParameter);
    }
}


/************************************************************************
 *                        ccxMngr_getCCXv4QosIElements                  *
 ************************************************************************
DESCRIPTION: This function resets the admission state variables as required
                for non-QoS mode and configures the Tx module.

INPUT:      Handler.
             

OUTPUT:     IEs buffer, length of the added IE

RETURN:     

************************************************************************/

TI_STATUS ccxMngr_getCCXQosIElements(TI_HANDLE hCcxMngr, TI_UINT8 *pQosIe, TI_UINT8 *pLen)
{
    TI_UINT32 ieLen;
    ccxMngr_t *pCcxMngr;

    pCcxMngr = (ccxMngr_t *)hCcxMngr;

    /* 1. Set tspec parameters for voice in candidateTspecInfo table and form IE */
    ieLen = qosMngr_buildTSPec(pCcxMngr->hQosMngr, USER_PRIORITY_6, pQosIe);
    pQosIe += (TI_UINT8)ieLen;
    *pLen = (TI_UINT8)ieLen;
    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getCCXQosIElements: voice IE %d \n", ieLen);

    /* 2. Form TSRS IE */
    ccxMngr_buildTsrsIE(0, pQosIe, &ieLen);
    pQosIe += (TI_UINT8)ieLen;
    *pLen += (TI_UINT8)ieLen;
    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getCCXQosIElements: TSRS IE %d \n", ieLen);

    /* 3. If exists, set signalling parameters for voice in candidateTspecInfo table and form IE */
    ieLen = qosMngr_buildTSPec(pCcxMngr->hQosMngr, USER_PRIORITY_4, pQosIe);
    pQosIe += (TI_UINT8)ieLen;
    *pLen += (TI_UINT8)ieLen;
    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_getCCXQosIElements: signalling IE %d \n", ieLen);

    return TI_OK;
}

/*****************************************************************************
**         Private Function section                                      **
*****************************************************************************/

/************************************************************************
 *                        ccxMngr_buildTsrsIE                           *
 ************************************************************************
DESCRIPTION: build a Traffic Stream Rate Set IE for the association request
                                                                                                   
INPUT:      -

OUTPUT:     len                      - the tspec frame len              

RETURN:     TI_OK on success, TI_NOK otherwise
************************************************************************/
static void ccxMngr_buildTsrsIE(TI_UINT8 tsid, TI_UINT8 *pDataBuff, TI_UINT32 *len)
{
    dot11_TSRS_STA_IE_t *dot11_TSRS_IE = (dot11_TSRS_STA_IE_t*)pDataBuff;

    dot11_TSRS_IE->hdr[0] =   WME_TSPEC_IE_ID;
    dot11_TSRS_IE->hdr[1] = sizeof(dot11_TSRS_STA_IE_t) - 2;

    dot11_TSRS_IE->OUI[0] = 0x00;
    dot11_TSRS_IE->OUI[1] = 0x40;
    dot11_TSRS_IE->OUI[2] = 0x96;
    dot11_TSRS_IE->oui_type = TS_RATE_SET_OUI_TYPE;
    dot11_TSRS_IE->tsid = tsid;
    dot11_TSRS_IE->tsNominalRate = WME_TSPEC_DEFAULT_NOMINAL_RATE;

    *len = sizeof(dot11_TSRS_STA_IE_t); 
}

/**
 * \brief ccxMngr_cckmTimerExpired - Called when CCKM timer expires. This happens when CCKM response
 * didn't arrive.
 *      In this case indicate that CCKM is not used, and call roam reply,
 *  to continue Roaming procedure.  
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static void ccxMngr_cckmTimerExpired(TI_HANDLE hCcxMngr, TI_BOOL bTwdInitOccured)
{
    ccxMngr_t           *pCcxMngr;
    if (hCcxMngr == NULL)
    {
        return;
    }
    pCcxMngr = (ccxMngr_t*)hCcxMngr;

    TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_cckmTimerExpired \n");

    pCcxMngr->useCckmFastRoaming = TI_FALSE;
    /* indicate that Roaming reply was received */
    apConn_RoamHandoffFinished(pCcxMngr->hAPConn);
}

/**
 * \brief ccxMngr_recvIappInformationReport - Called when Information IAPP packet is received.
 *      The packet is parsed and the list of channels is saved.
 *  When the station exits the CONNECTED state the list of channels is zeroed.
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) iappPacket - the recieved IAPP packet \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static TI_STATUS ccxMngr_recvIappInformationReport(TI_HANDLE hCcxMngr, TI_UINT8 *iappPacket)
{
    ccxMngr_t                               *pCcxMngr;
    TI_UINT8                                *pRecvIappPacket;
    IAPP_msg_header_t                       *pIAPP_header;
    IAPP_Report_Information_msg_body_t      *pIAPP_Report_Information_msg_body;
    TI_UINT16                               iappLength, length;
    paramInfo_t                             param;
    TI_BOOL                                 currentAP;
    TMacAddr                                currentAPMac=DUMMY_MAC_ADDRESS;
    TSsid                                   currentSsid;
    TI_STATUS                               status;


    if ((hCcxMngr==NULL) || (iappPacket==NULL))
    {
        return TI_NOK;
    }
    pCcxMngr = (ccxMngr_t*) hCcxMngr;
    
    /* save the current SSID */
    param.paramType = SME_DESIRED_SSID_ACT_PARAM;
    status = sme_GetParam (pCcxMngr->hSme, &param);
    if (status==TI_OK)
    {
        currentSsid.len = param.content.smeDesiredSSID.len;
        os_memoryCopy(pCcxMngr->hOs, (void*)currentSsid.str, (void*)param.content.smeDesiredSSID.str, param.content.smeDesiredSSID.len); 
    }

    /* save the current BSSID */
    param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
    status = ctrlData_getParam(pCcxMngr->hCtrlData, &param);
    if (status == TI_OK)
    {
        MAC_COPY (currentAPMac, param.content.ctrlDataCurrentBSSID); 
    }

    pIAPP_header = (IAPP_msg_header_t*)iappPacket;

    pCcxMngr->neighborAPList.numOfEntries = 0;
    
    iappLength = HTOWLANS(pIAPP_header->length);
    TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_recvIappInformationReport =\n");
    TRACE_INFO_HEX(pCcxMngr->hReport, iappPacket, iappLength); 

    if ((iappLength<sizeof(IAPP_msg_header_t)) || (iappLength>(MAX_AP_INFORMATION_REPORT_LIST*sizeof(IAPP_Report_Information_msg_body_t))))
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_recvIappInformationReport length too small/big = %d\n", iappLength);
        return TI_OK;
    }
    length = sizeof(IAPP_msg_header_t);
    pRecvIappPacket = iappPacket + length;
    
    while (length<iappLength)
    {
        TI_UINT16      channel, deltaLength;

        /* set the received site as priority site for roaming */
        pIAPP_Report_Information_msg_body = (IAPP_Report_Information_msg_body_t*)pRecvIappPacket;
        TRACE_INFO_HEX(pCcxMngr->hReport, pRecvIappPacket, HTOWLANS(pIAPP_Report_Information_msg_body->length));

        currentAP = MAC_EQUAL (currentAPMac, pIAPP_Report_Information_msg_body->previousApMac);
        /* check if the sent SSID matches the desired one*/
        if (!currentAP &&
            !os_memoryCompare(pCcxMngr->hOs, (void*)currentSsid.str, (void*)pIAPP_Report_Information_msg_body->ssid, currentSsid.len))
        {   /* The SSID matches, save the channel */
            MAC_COPY (param.content.siteMgrPrioritySite.siteMacAddress, pIAPP_Report_Information_msg_body->previousApMac);
            channel = HTOWLANS(pIAPP_Report_Information_msg_body->channel);
            /* save the channel in list of channels */
            ccxMngr_saveNeighborAps(pCcxMngr, pIAPP_Report_Information_msg_body);


            TRACE7(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_recvIappInformationReport, MAC 0x%x-0x%x-0x%x-0x%x-0x%x-0x%x, channel=%d \n", pIAPP_Report_Information_msg_body->previousApMac[0], pIAPP_Report_Information_msg_body->previousApMac[1], pIAPP_Report_Information_msg_body->previousApMac[2], pIAPP_Report_Information_msg_body->previousApMac[3], pIAPP_Report_Information_msg_body->previousApMac[4], pIAPP_Report_Information_msg_body->previousApMac[5], channel);

        }
        deltaLength = HTOWLANS(pIAPP_Report_Information_msg_body->length)+4;/*+sizeof(pIAPP_Report_Information_msg_body->tag_adjacentApReport)+sizeof(pIAPP_Report_Information_msg_body->length);*/
        length += deltaLength;
        pRecvIappPacket += deltaLength;
        TRACE3(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_recvIappInformationReport, deltaLength=%d, length=%d, iappLength=%d \n", deltaLength,length,iappLength);
    }
    if (pCcxMngr->neighborAPList.numOfEntries>0)
    {
        apConn_updateNeighborAPsList(pCcxMngr->hAPConn, &(pCcxMngr->neighborAPList));
    }
    /* search for the maximum number of APs in channel */
    /* get the band type in order to decide on which channels to earsch */
    return TI_OK;
}

/**
 * \brief ccxMngr_LinkTestRetriesUpdate - Called when Link Test packet is completed transmit.
 *      In order to send the number of retries for the next link test packet
 *
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) ackFailures - retries number of the Tx packet \n
 * Return Value: NONE.\n
 */
void ccxMngr_LinkTestRetriesUpdate(TI_HANDLE hCcxMngr, TI_UINT8 ackFailures)
{
    ccxMngr_t               *pCcxMngr = (ccxMngr_t*)hCcxMngr;

    pCcxMngr->PriviousRetries += ackFailures;

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_LinkTestRetriesUpdate: retries number of Link Test packet= %d\n", pCcxMngr->PriviousRetries);
}


/**
 * \brief ccxMngr_recvLinkTest - Called when Link Test packet is received.
 *      The packet is parsed and respond Link Test packet will be send.
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) hCcxMngr - ccxMngr Object handler \n
 * 2) iappPacket - the recieved IAPP packet \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static TI_STATUS ccxMngr_LinkTestResponse(TI_HANDLE hCcxMngr, TI_UINT8 *iappPacket)
{
    ccxMngr_t               *pCcxMngr;
    IAPP_Link_Test_msg_t    IAPP_Link_Test_msg;
    IAPP_msg_header_t       *pckHdr;
    TI_UINT32               dataLen, iappMsgLen;
    static TI_UINT16        frameNumberIndex = 0;
    TMacAddr                dummyAdd=DUMMY_MAC_ADDRESS;


    if ((hCcxMngr==NULL) || (iappPacket==NULL))
    {
        return TI_NOK;
    }

    pCcxMngr = (ccxMngr_t*) hCcxMngr;

    pckHdr = (IAPP_msg_header_t*)iappPacket;

    TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_LinkTestRespond: pckHdr->length = 0x%x\n", pckHdr->length);

    dataLen = HTOWLANS(pckHdr->length);
    iappMsgLen = IAPP_PACKET_HEADER_LEN + IAPP_LINK_TEST_MSG_BODY_LEN + dataLen;

    /*Make sure the subtype of the packet is RM request (= 0x01)*/
    if(pckHdr->funcCode != RM_REQUEST)
    {
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_LinkTestRespond: Error, subType is invalid.");
        return TI_NOK;
    }

    IAPP_Link_Test_msg.IAPP_Link_Test_msg_body.frameNumber = WLANTOHS (frameNumberIndex);

    /* Print for Debug */
    TRACE25(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_LinkTestRespond:\nReceived Frame Hdr:\nSNAP Heaser:%x %x %x %x %x %x %x %x\nLength: %d\nType: %x, Sub-Type: %x\nDest. Mac Address: %x %x %x %x %x %x\nSource Mac Address: %x %x %x %x %x %x\nFrame Number: %d\nDataLen = %d\n", pckHdr->snapHeader[0], pckHdr->snapHeader[1], pckHdr->snapHeader[2], pckHdr->snapHeader[3], pckHdr->snapHeader[4], pckHdr->snapHeader[5], pckHdr->snapHeader[6], pckHdr->snapHeader[7], iappMsgLen, pckHdr->messageType, pckHdr->funcCode, pckHdr->destinationMacAddress[0], pckHdr->destinationMacAddress[1], pckHdr->destinationMacAddress[2], pckHdr->destinationMacAddress[3], pckHdr->destinationMacAddress[4], pckHdr->destinationMacAddress[5], pckHdr->sourceMacAddress[0], pckHdr->sourceMacAddress[1], pckHdr->sourceMacAddress[2], pckHdr->sourceMacAddress[3], pckHdr->sourceMacAddress[4], pckHdr->sourceMacAddress[5], WLANTOHS( *(iappPacket + sizeof(IAPP_msg_header_t)) ), dataLen);

    /* in order to add the destination Add at ccxMngr_sendIAPPPacket() */
    MAC_COPY (pckHdr->destinationMacAddress, dummyAdd);

    os_memoryCopy(pCcxMngr->hOs, &IAPP_Link_Test_msg, iappPacket, iappMsgLen);

    IAPP_Link_Test_msg.IAPP_Link_Test_header.funcCode = RM_REPORT;

    frameNumberIndex++;

    /* send the privious retries of link test packet (use offset and not struct-field to avoid pack problem!) */
    *( ( (TI_UINT8 *)(&IAPP_Link_Test_msg.IAPP_Link_Test_msg_body) ) + 8) = pCcxMngr->PriviousRetries;

    pCcxMngr->PriviousRetries = 0;

    /* send the sigStDbm (use offset and not struct-field to avoid pack problem!) */
    *( ( (TI_UINT8 *)(&IAPP_Link_Test_msg.IAPP_Link_Test_msg_body) ) + 9) = 256 - pCcxMngr->pRxAttr->Rssi;

    /* send the IAPP packets */
    return ccxMngr_sendIAPPPacket(pCcxMngr, 
                                  &IAPP_Link_Test_msg.IAPP_Link_Test_header, 
                                  (TI_UINT8 *)&(IAPP_Link_Test_msg.IAPP_Link_Test_msg_body), 
                                  iappMsgLen);
}

/**
 * \brief ccxMngr_updatePrevAssocParams - Called when entering CONNECTED state to update the previous
 * association AP parameters, required for the Information IAPP packet.
 *      The Report Information IAPP packet includes the previous AP:
 *  - Channel number
 *  - SSID length
 *  - SSID 
 *  - Seconds the client has been disassociated
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) pCcxMngr - ccxMngr Object handler \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static void  ccxMngr_updatePrevAssocParams(ccxMngr_t *pCcxMngr)
{
    paramInfo_t     param;
    TI_STATUS       status;

    param.paramType = SME_DESIRED_SSID_ACT_PARAM;
    status = sme_GetParam (pCcxMngr->hSme, &param);
    if (status==TI_OK)
    {
        pCcxMngr->prevApParams.prevAssocSsid.len = (param.content.smeDesiredSSID.len>32) ? 32 : param.content.smeDesiredSSID.len;
        os_memoryCopy(pCcxMngr->hOs, (void*)pCcxMngr->prevApParams.prevAssocSsid.str, (void*)param.content.smeDesiredSSID.str, param.content.smeDesiredSSID.len); 
    }
    param.paramType = SITE_MGR_CURRENT_CHANNEL_PARAM;
    status = siteMgr_getParam(pCcxMngr->hSiteMgr, &param);
    if (status==TI_OK)
    {
        pCcxMngr->prevApParams.prevAssocChannel = param.content.siteMgrCurrentChannel & 0x00FF;
    }

    param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
    status = ctrlData_getParam(pCcxMngr->hCtrlData, &param);
    if (status == TI_OK)
    {
        MAC_COPY (pCcxMngr->prevApParams.prevAssocBssid, param.content.ctrlDataCurrentBSSID);
    }

}

/**
 * \brief ccxMngr_saveNeighborAps - Called when Information IAPP packet is received.
 *      The channels list is saved as a bit map of channels for scan.
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) pCcxMngr - ccxMngr Object handler \n
 * 2) channelNumber - the channel number to be saved \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static void ccxMngr_saveNeighborAps(ccxMngr_t *pCcxMngr, IAPP_Report_Information_msg_body_t  *pIAPP_Report_Information_msg_body)
{
    paramInfo_t     param;
    TI_UINT16          channel;
	TI_STATUS		status;

    if ((pCcxMngr==NULL) || (pIAPP_Report_Information_msg_body==NULL))
    {
        return;
    }

    param.paramType = SME_DESIRED_SSID_ACT_PARAM;
    status = sme_GetParam (pCcxMngr->hSme, &param);
    if ((param.content.smeDesiredSSID.len==0) ||
        os_memoryCompare(pCcxMngr->hOs, (void*)param.content.smeDesiredSSID.str, (void*)pIAPP_Report_Information_msg_body->ssid, param.content.smeDesiredSSID.len))
    {   /* SSIDs do not match, ignore the BSSID */
        TRACE0(pCcxMngr->hReport, REPORT_SEVERITY_INFORMATION, "ccxMngr_saveNeighborAps - SSIDs do not match desired SSID=, ssid=\n");
        return;
    }

    channel = HTOWLANS(pIAPP_Report_Information_msg_body->channel);

    if (channel == 0)
    {
            TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_saveNeighborAps - bad channelNumber=%d\n", channel);
            return;
    }
    /* Get desired SSID, compare to given SSID, if not equal, discard the BSSID */
    if (pCcxMngr->neighborAPList.numOfEntries >= MAX_AP_INFORMATION_REPORT_LIST)
    {
        TRACE1(pCcxMngr->hReport, REPORT_SEVERITY_ERROR, "ccxMngr_saveNeighborAps - bad numberOfAps=%d\n",pCcxMngr->neighborAPList.numOfEntries);
        return;
    }

    pCcxMngr->neighborAPList.APListPtr[pCcxMngr->neighborAPList.numOfEntries].channel = (TI_UINT8)channel;
    MAC_COPY (pCcxMngr->neighborAPList.APListPtr[pCcxMngr->neighborAPList.numOfEntries].BSSID, 
              pIAPP_Report_Information_msg_body->previousApMac);
    if (channel > MAX_CHANNEL_IN_BAND_2_4)
    {
        pCcxMngr->neighborAPList.APListPtr[pCcxMngr->neighborAPList.numOfEntries].band = RADIO_BAND_5_0_GHZ;
    }
    else
    {
        pCcxMngr->neighborAPList.APListPtr[pCcxMngr->neighborAPList.numOfEntries].band = RADIO_BAND_2_4_GHZ;
    }
    pCcxMngr->neighborAPList.numOfEntries++;


}



/**
 * \brief ccxMngr_reportCcxEnableStatusChange - Called when ccx enable status was changed.
 *      All registered procedures for this change are called.
 * Currently, this is not used, since no Module registers to this service.
 *
 * Function Scope \e Private.\n
 * Parameters:\n
 * 1) pCcxMngr - ccxMngr Object handler \n
 * Return Value: TI_OK or TI_NOK.\n
 */
static TI_STATUS ccxMngr_reportCcxEnableStatusChange(ccxMngr_t *pCcxMngr)
{
    TI_UINT8                                   ccxEnableIndex;
    ccxEnableStatusRegistrationTable_t      *ccxEnableStatusRegistrationProc;   
    
    if (pCcxMngr==NULL)
    {
        return TI_NOK;
    }
    /* look for handlers that were registered for the CCX Enable status change */
    for (ccxEnableIndex=0; ccxEnableIndex<pCcxMngr->ccxEnableStatusRegistrationCounter; ccxEnableIndex++)
    {
        ccxEnableStatusRegistrationProc = &pCcxMngr->ccxEnableStatusRegistrationTable[ccxEnableIndex];

        if (ccxEnableStatusRegistrationProc != NULL)
        {
            ccxEnableStatusRegistrationProc->ccxEnableStatusRegistrationProcedure(ccxEnableStatusRegistrationProc->handler, &pCcxMngr->ccxEnable);
        }
    }
    return TI_OK;

}

static TI_UINT8 ccxMngr_translateAssocRoamingTriggerToReasonCode(ccxMngr_t *pCcxMngr, apConn_roamingTrigger_e roamingTrigger)
{
    if (pCcxMngr==NULL)
    {
        return TI_NOK;
    }

	switch (roamingTrigger)
	{
	case ROAMING_TRIGGER_LOW_QUALITY_FOR_BG_SCAN:
	case ROAMING_TRIGGER_LOW_SNR:
	case ROAMING_TRIGGER_LOW_QUALITY:
	case ROAMING_TRIGGER_MAX_TX_RETRIES:
	case ROAMING_TRIGGER_BSS_LOSS:
	case ROAMING_TRIGGER_SWITCH_CHANNEL:
		return ASSOC_REASON_NORMAL_POOR_LINK;

	case ROAMING_TRIGGER_NORMAL_QUALITY_FOR_BG_SCAN:
	case ROAMING_TRIGGER_LOW_TX_RATE:
		return ASSOC_REASON_NORMAL_ROAM_BETTER_AP;

	case ROAMING_TRIGGER_AP_DISCONNECT:
		return ASSOC_REASON_DEAUTH_FROM_AP;

	case ROAMING_TRIGGER_SECURITY_ATTACK:
		return ASSOC_REASON_UNSPECIFIED;

	default:
		return ASSOC_REASON_UNSPECIFIED;
	}

}

#endif /* CCX_MODULE_INCLUDED */
