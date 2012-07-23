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
/** \file ccxMngr.h
 *  \brief CCX Manager API
 *
 *  \see ccxMngr.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  CCX Manager                                                   *
 *   PURPOSE: CCX Manager API                                               *
 *                                                                          *
 ****************************************************************************/

#ifndef _CCX_MNGR_H_
#define _CCX_MNGR_H_


#ifdef CCX_MODULE_INCLUDED

#include "paramOut.h"
#include "802_11Defs.h"
#include "bssTypes.h"
#include "apConn.h"
#include "DataCtrl_Api.h"
#include "DrvMainModules.h"


/* Constants */
#define IAPP_PACKET_HEADER_LEN                  24

#define IAPP_PACKET_SNAP                        { 0xAA, 0xAA, 0x03, 0x00, 0x40, 0x96, 0x00, 0x00 }
#define IAPP_CISCO_AIRONET_OUI                  { 0x00, 0x40, 0x96, 0x00} 
#define MAX_OBJECTS_REGISTERS_FOR_IAPP_PACKET   10
#define MAX_OBJECTS_REGISTERS_FOR_CCX_ENABLE    10

#define MAX_AP_INFORMATION_REPORT_LIST      30

#define IAPP_LINK_TEST_MSG_BODY_LEN         12
#define IAPP_LINK_TEST_DATA_MAX_SIZE        1400

#define CCX_CCKM_OUI_TYPE                   (0x0)
#define CCX_RADIO_MNG_CAPABILITIES_OUI_TYPE (0x1)
#define CCX_VERSION_NUMBER_OUI_TYPE         (0x03)
#define CCX_SSIDL_OUI_TYPE                  (0x05)
#define CCX_TRAFFIC_STREAM_METRICS_OUI_TYPE TS_METRIX_OUI_TYPE
#define CCX_TSRS_OUI_TYPE                   TS_RATE_SET_OUI_TYPE
#define CCX_MSDU_LIFE_TIME_OUI_TYPE         EDCA_LIFETIME_OUI_TYPE
#define CCX_EXT_CAPABILITIES_OUI_TYPE       (0x0B)

#define DUMMY_MAC_ADDRESS         {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

#define     RM_REQUEST                          0x01
#define     RM_REPORT                           0x81
#define     RM_REPORT_IE_ID                     39
#define     IAPP_HDR_LEN                        16
#define     IAPP_SNAP_HDR_LEN                   8

/* External data definitions */

/* Enumerations */
typedef enum 
{
    CCX_CCKM_EXISTS  = 0x01,
    CCX_NEIGHBOR_APS = 0x02

} ECcxParam;


/* CCX IAPP packet types as they appear in CCX V2 spec */
typedef enum 
{
    IAPP_REPORT_INFORMATION = 0x30,
    IAPP_FIRST_TYPE         = IAPP_REPORT_INFORMATION,
    IAPP_RADIO_MEASUREMENT  = 0x32,
    IAPP_LINK_TEST          = 0x41,
    IAPP_LAST_TYPE, 
    IAPP_ROGUE_AP           = 0x40 /* This is a tx only packet */
    
} iappType_e;

#define IAPP_TS_METRICS_REPORT_IE_ID        0x27
#define IAPP_TS_METRICS_REPORT_IE_LENGTH    25

/* Types of assoc sucess and failures, with a specific reason 
for assoc failure due to Rogue AP */
typedef enum 
{
    CCX_ASSOC_OK,
    CCX_DISASSOC,
    CCX_ROGUE
} assocStatus_e;

/* Typedefs */

typedef struct _ccxMngr_t   ccxMngr_t;

/* CCX IAPP Information packet header as it appears in CCX V2 S32 */
typedef struct 
{
    TI_UINT8           snapHeader[WLAN_SNAP_HDR_LEN];
    TI_UINT16          length;
    TI_UINT8           messageType;
    TI_UINT8           funcCode;
    TMacAddr           destinationMacAddress;
    TMacAddr           sourceMacAddress;
} IAPP_msg_header_t;

/* CCX IAPP List Test body as it appears in CCX V4 */
typedef struct 
{
    TI_UINT16 frameNumber;
    TI_UINT32 timesStamp;
    TI_UINT8  rawSigQlt;        /* raw signal quality */
    TI_UINT8  rawSigSt;         /* raw signal strength */
    TI_UINT8  retries;          /* num of retries last time */
    TI_UINT8  sigStDbm;         /* signal strength in dbm */
    TI_UINT8  sigQltPct;        /* signal quality in percentage */
    TI_UINT8  sigStPct;         /* signal strength in percentage */
    TI_UINT8  data[IAPP_LINK_TEST_DATA_MAX_SIZE];   
}  IAPP_Link_Test_msg_body_t;

/* CCX IAPP List Test packet body & header as it appears in CCX V4 */
typedef struct 
{
    IAPP_msg_header_t                   IAPP_Link_Test_header;
    IAPP_Link_Test_msg_body_t           IAPP_Link_Test_msg_body; 
}  IAPP_Link_Test_msg_t;

/* a function prototype for registrating to recv/send IAPP packets */
typedef TI_STATUS (*iappParsingRegistrationProcedure_t)(TI_HANDLE handler, TI_UINT8 *iappPacket);
typedef struct 
{
    TI_HANDLE                           handler;
    iappParsingRegistrationProcedure_t  iappParsingRegistrationProcedure;
} iappParsingRegistrationTable_t;

/* a function prototype for registrating to recv notification when CCX status
    (enable/disable) is changed */
typedef TI_STATUS (*ccxEnableStatusRegistrationProcedure_t)(TI_HANDLE handler, ccxMngr_mode_t *enableStatus);
typedef struct 
{
    TI_HANDLE                               handler;
    ccxEnableStatusRegistrationProcedure_t  ccxEnableStatusRegistrationProcedure;
} ccxEnableStatusRegistrationTable_t;

/* The previous associated AP parameters that are required to be sent in
    the IAPP information packet */
typedef struct 
{
    TMacAddr            prevAssocBssid;
    TI_UINT16           prevAssocChannel;
    TSsid               prevAssocSsid;
}  prevApParams_t;



/* Structures */
/* The CCX Manager Object type */
struct _ccxMngr_t
{
    /* CCX data that can be set/get Externally */
    ccxMngr_mode_t             ccxEnable;
    TI_UINT32                  ccxConfiguration;

    /* CCX internal data */
    TI_BOOL                    useCckmFastRoaming;
    TI_UINT8                   ccxSupportedVersion;
    TI_UINT8                   apCandidateCcxVersion;
    TI_BOOL                    cckmAkmExists;
    TI_BOOL                    useCckmAkm;
    prevApParams_t             prevApParams;
    TI_UINT32                  disAssociateTime;
    TI_BOOL                    sendIappInfo;
    TI_UINT32                  cckmRetriesCounter; 
    TI_BOOL                    associated;

    /* CCKM reassoc IE */
    TI_UINT8                   associationRequestIE[255];
    TI_UINT8                   associationRequestIELength;
                        
    /* table of registered modules for recv/send IAPP packets */
    iappParsingRegistrationTable_t      iappPacketRecvRegisterObjectsTable[IAPP_LAST_TYPE-IAPP_FIRST_TYPE][MAX_OBJECTS_REGISTERS_FOR_IAPP_PACKET];
    TI_UINT8                               iappPacketRecvRegisterObjectsCounter[IAPP_LAST_TYPE-IAPP_FIRST_TYPE];
    
    /* table of registered modules for recv/send CCX enable status */
    ccxEnableStatusRegistrationTable_t  ccxEnableStatusRegistrationTable[MAX_OBJECTS_REGISTERS_FOR_CCX_ENABLE];
    TI_UINT8                               ccxEnableStatusRegistrationCounter;
    
    neighborAPList_t        neighborAPList;

    /* CCKM timer */
    TI_HANDLE               hCckmTimer;

    /* Handles to other objects */
    TI_HANDLE               hReport;
    TI_HANDLE               hOs;
    TI_HANDLE               hRsn;
    TI_HANDLE               hRogueAp;
    TI_HANDLE               hCtrlData;
    TI_HANDLE               hTxCtrl;
    TI_HANDLE               hTxMgmtQ;
    TI_HANDLE               hSiteMgr;
    TI_HANDLE               hSme;
    TI_HANDLE               hAPConn;
    TI_HANDLE               hEvHandler;
    TI_HANDLE               hMeasurementMngr;
    TI_HANDLE               hQosMngr;
    TI_HANDLE               hTimer;
    TI_HANDLE               hTWD;

    /* debug */
    TI_UINT32                  IappSentInfoTS;

    /* link test */
    TI_UINT8                    PriviousRetries;
    TRxAttr*                pRxAttr;

};


/* External functions definitions */

/* Function prototypes */

TI_HANDLE ccxMngr_create(TI_HANDLE hOs);

void      ccxMngr_init (TStadHandlesList *pStadHandles);

TI_STATUS ccxMngr_SetDefaults (TI_HANDLE hCcxMngr, ccxMngrParams_t *pCcxMngrParams);

TI_STATUS ccxMngr_unload(TI_HANDLE hCcxMngr);

TI_STATUS ccxMngr_setParam(TI_HANDLE hCcxMngr, paramInfo_t *pParam);

TI_STATUS ccxMngr_getParam(TI_HANDLE hCcxMngr, paramInfo_t *pParam);

TI_STATUS ccxMngr_recvIAPPPacket(TI_HANDLE hCcxMngr, void *pBuffer, TRxAttr* pRxAttr);

TI_BOOL ccxMngr_isIappPacket(TI_HANDLE hCcxMngr, void *pBuffer);

TI_STATUS ccxMngr_sendIAPPPacket(TI_HANDLE hCcxMngr, IAPP_msg_header_t *IAPP_msg_header, TI_UINT8 *pPacket, TI_UINT32 length);

TI_UINT8 ccxMngr_parseCcxVer(TI_HANDLE hCcxMngr, TI_UINT8 *pIeBuffer, TI_UINT16 length);

TI_STATUS ccxMngr_startCckm (TI_HANDLE hCcxMngr, TMacAddr *macAddress, TI_UINT8 *tsfTimeStamp);

TI_STATUS ccxMngr_getCckmInfoElement(TI_HANDLE hCcxMngr, TI_UINT8 *pCckmIe, TI_UINT8 *pCckmIeLen);

TI_STATUS ccxMngr_registerForRecvIappPacket(TI_HANDLE hCcxMngr, iappParsingRegistrationTable_t iappParsingRegistration, iappType_e iappType);

TI_STATUS ccxMngr_updateIappInformation(TI_HANDLE hCcxMngr, assocStatus_e assocStatus);

TI_STATUS ccxMngr_getCCXVersionInfoElement(TI_HANDLE hCcxMngr, TI_UINT8 *pCcxVerIe, TI_UINT8 *pCcxVerIeLen);

TI_STATUS ccxMngr_rogueApDetected(TI_HANDLE hCcxMngr, EAuthStatus failureReason);

TI_STATUS ccxMngr_registerForCcxEnableStatus(TI_HANDLE hCcxMngr, ccxEnableStatusRegistrationTable_t ccxEnableStatusRegistration);

TI_STATUS ccxMngr_getCurMAcAddress(TI_HANDLE hCcxMngr, TMacAddr *macAddress);

void ccxMngr_setCCXQoSParams(TI_HANDLE hCcxMngr, CCXv4IEs_t *ies, TI_UINT8 ac);

TI_STATUS ccxMngr_getCCXQosIElements(TI_HANDLE hCcxMngr, TI_UINT8 *pQosIe, TI_UINT8 *pLen);
void ccxMngr_buildCcxTS_IEs(TI_HANDLE hCcxMngr, TI_UINT8 *pDataBuff, TI_UINT32 *len, TI_UINT8 tid);

void ccxMngr_LinkTestRetriesUpdate(TI_HANDLE hCcxMngr, TI_UINT8 ackFailures);

#endif /* CCX_MODULE_INCLUDED*/
#endif /*  _CCX_MNGR_H_*/

