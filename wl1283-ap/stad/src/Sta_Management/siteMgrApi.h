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
/** \file siteMgrApi.h
 *  \brief site manager module API
 *
 *  \see siteMgr.c
 */

/***************************************************************************/
/*                                                                                                  */
/*    MODULE:   siteMgrApi.h                                                                */
/*    PURPOSE:  site manager module API                                         */
/*                                                                                                  */
/***************************************************************************/
#ifndef __SITE_MGR_API_H__
#define __SITE_MGR_API_H__

#include "tidef.h"
#include "paramOut.h"
#include "802_11Defs.h"
#include "mlmeApi.h"
#include "siteHash.h"
#include "ScanCncn.h"
#include "bssTypes.h"
#include "DrvMainModules.h"
#include "scanResultTable.h"


#define SPECIAL_BG_CHANNEL              14

/* Rx quality triggers indexes */
typedef enum
{
    TRIGGER_EVENT_LOW_RSSI   = 0,
    TRIGGER_EVENT_LOW_SNR    = 1,
    TRIGGER_EVENT_HIGH_TX_PW = 2,
    TRIGGER_EVENT_LOW_TX_PW  = 3,
    TRIGGER_EVENT_BG_SCAN    = 4,
    TRIGGER_EVENT_USER_0     = 5,
    TRIGGER_EVENT_USER_1     = 6,
    TRIGGER_EVENT_MAX        = 7

}ETriggerEventIndex;


typedef enum
{
    /* SiteMgr section */   
    SITE_MGR_POWER_CONSTRAINT_PARAM     = 0x01,
    SITE_MGR_BEACON_INTERVAL_PARAM      = 0x02,
    SITE_MGR_SITE_CAPABILITY_PARAM      = 0x05,
    SITE_MGR_RGSTRY_BASIC_RATE_SET_MASK = 0x08,
    SITE_MGR_BEACON_RECV                = 0x09,
    SITE_MGR_DTIM_PERIOD_PARAM          = 0x0A,

    /* Previous Primary Site */
    SITE_MGR_PREV_SITE_BSSID_PARAM      = 0x0B,
    SITE_MGR_PREV_SITE_SSID_PARAM       = 0x0C,
    SITE_MGR_PREV_SITE_CHANNEL_PARAM    = 0x0D,
    SITE_MGR_DESIRED_RSSI_GAP_THR_PARAM = 0x0E,
    SITE_MGR_PRIORITY_PARAM             = 0x0F

} ESiteMgrParam;


/* Site manager interface functions prototypes */

TI_HANDLE siteMgr_create(TI_HANDLE hOs);

void      siteMgr_init (TStadHandlesList *pStadHandles);

TI_STATUS siteMgr_SetDefaults (TI_HANDLE       hSiteMgr,
                               siteMgrInitParams_t     *pSiteMgrInitParams);

TI_STATUS siteMgr_unLoad(TI_HANDLE hSiteMgr);

TI_STATUS siteMgr_setParam(TI_HANDLE        hSiteMgr,
                        paramInfo_t     *pParam);

TI_STATUS siteMgr_getParamWSC(TI_HANDLE hSiteMgr, TIWLN_SIMPLE_CONFIG_MODE *wscParam);

TI_STATUS siteMgr_getParam(TI_HANDLE        hSiteMgr, 
                        paramInfo_t     *pParam);

TI_STATUS siteMgr_join(TI_HANDLE    hSiteMgr);

TI_STATUS siteMgr_removeSelfSite(TI_HANDLE  hSiteMgr);

TI_STATUS siteMgr_disSelectSite(TI_HANDLE   hSiteMgr);

TI_STATUS systemConfig(siteMgr_t *pSiteMgr);

TI_STATUS siteMgr_start(TI_HANDLE   hSiteMgr);

TI_STATUS siteMgr_stop(TI_HANDLE    hSiteMgr);

TI_STATUS siteMgr_resetSiteTable(TI_HANDLE  hSiteMgr, siteTablesParams_t*   pSiteTableParams);

TI_STATUS siteMgr_updatePrimarySiteFailStatus(TI_HANDLE hSiteMgr, 
                                           TI_BOOL bRemoveSite);

TI_BOOL siteMgr_isCurrentBand24(TI_HANDLE  hSiteMgr);

TI_STATUS pbccAlgorithm(TI_HANDLE hSiteMgr);

TI_STATUS siteMgr_assocReport(TI_HANDLE hSiteMgr, TI_UINT16 capabilities, TI_BOOL bCiscoAP);

void siteMgr_setCurrentTable(TI_HANDLE hSiteMgr, ERadioBand radioBand);

void siteMgr_updateRates(TI_HANDLE hSiteMgr, TI_BOOL dot11a, TI_BOOL updateToOS);

void siteMgr_bandParamsConfig(TI_HANDLE hSiteMgr, TI_BOOL updateToOS);

void siteMgr_ConfigRate(TI_HANDLE hSiteMgr);

TI_STATUS siteMgr_getWMEParamsSite(TI_HANDLE hSiteMgr, dot11_ACParameters_t **pWME_ACParameters_t);

TI_STATUS siteMgr_setWMEParamsSite(TI_HANDLE hSiteMgr,dot11_WME_PARAM_t *pDot11_WME_PARAM);

void siteMgr_IsERP_Needed(TI_HANDLE hSiteMgr,TI_BOOL *useProtection,TI_BOOL *NonErpPresent,TI_BOOL *barkerPreambleType);

TI_STATUS siteMgr_CopyToPrimarySite (TI_HANDLE hSiteMgr, TSiteEntry *pCandidate);

TI_STATUS siteMgr_overwritePrimarySite(TI_HANDLE hSiteMgr, bssEntry_t *newAP, TI_BOOL requiredToStorePrevSite);

void siteMgr_changeBandParams (TI_HANDLE hSiteMgr, ERadioBand radioBand);

TI_BOOL siteMgr_SelectRateMatch (TI_HANDLE hSiteMgr, TSiteEntry *pCurrentSite);

siteEntry_t *addSelfSite(TI_HANDLE hSiteMgr);

TI_STATUS siteMgr_updateSite(TI_HANDLE			hSiteMgr, 
						  TMacAddr		*bssid, 
						  mlmeFrameInfo_t	*pFrameInfo,
						  TI_UINT8				rxChannel,
                          ERadioBand       band,
						  TI_BOOL				measuring);

TI_STATUS siteMgr_IbssMerge(TI_HANDLE       hSiteMgr,
                          TMacAddr      	our_bssid,
						  TMacAddr      	new_bssid,
                          mlmeFrameInfo_t   *pFrameInfo,
                          TI_UINT8          rxChannel,
                          ERadioBand        band);

TI_STATUS siteMgr_saveProbeRespBuffer(TI_HANDLE hSiteMgr, TMacAddr	*bssid, TI_UINT8 *pProbeRespBuffer, TI_UINT32 length);

TI_STATUS siteMgr_saveBeaconBuffer(TI_HANDLE hSiteMgr, TMacAddr *bssid, TI_UINT8 *pBeaconBuffer, TI_UINT32 length);

void siteMgr_UpdatHtParams (TI_HANDLE hSiteMgr, siteEntry_t *pSite, mlmeFrameInfo_t *pFrameInfo);


#ifdef REPORT_LOG
void siteMgr_printPrimarySiteDesc(TI_HANDLE hSiteMgr );
#endif

#endif /* __SITE_MGR_API_H__ */
