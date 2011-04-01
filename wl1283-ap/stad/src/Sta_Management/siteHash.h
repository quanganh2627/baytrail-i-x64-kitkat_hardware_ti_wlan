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
/** \file siteHash.h
 *  \brief Hash & site table internal header file
 *
 *  \see siteHash.c
 */

/***************************************************************************/
/*                                                                          */
/*    MODULE:   siteHash.h                                                  */
/*    PURPOSE:  Hash & site table internal header file                      */
/*                                                                          */
/***************************************************************************/
#ifndef __SITE_MGR_H__
#define __SITE_MGR_H__

#include "tidef.h"
#include "paramOut.h"
#include "802_11Defs.h"
#include "DataCtrl_Api.h"
#include "scanResultTable.h"
#include "roleAP.h"


#define MIN_TX_SESSION_COUNT    1
#define MAX_TX_SESSION_COUNT    7

/* A site entry contains all the site attribute received in beacon and probes
    and data used to manage the site table and hash table */
typedef TSiteEntry siteEntry_t;

typedef struct
{
    TI_UINT8           numOfSites;
    TI_UINT8           maxNumOfSites;
    siteEntry_t        siteTable[MAX_SITES_BG_BAND];
}siteTablesParams_t;

/* This struct is seperated from the above struct (siteTablesParams_t) for memory optimizations */
typedef struct
{
    TI_UINT8           numOfSites;
    TI_UINT8           maxNumOfSites;
    siteEntry_t        siteTable[MAX_SITES_A_BAND];
}siteTablesParamsBandA_t;

/* Ths following structure is used to manage the sites */
typedef struct
{
    siteTablesParamsBandA_t   dot11A_sitesTables;
    siteTablesParams_t        dot11BG_sitesTables;
    siteTablesParams_t        *pCurrentSiteTable;
    siteEntry_t               *pPrimarySite;
    siteEntry_t               *pPrevPrimarySite;
} sitesMgmtParams_t;


/* Site manager handle */
typedef struct
{
    siteMgrInitParams_t *pDesiredParams;
    sitesMgmtParams_t   *pSitesMgmtParams;

    TI_HANDLE           hConn;
    TI_HANDLE           hSmeSm;
    TI_HANDLE           hCtrlData;
    TI_HANDLE           hRxData;
    TI_HANDLE           hTxCtrl;
    TI_HANDLE           hRsn;
    TI_HANDLE           hAuth;
    TI_HANDLE           hAssoc;
    TI_HANDLE           hRegulatoryDomain;
    TI_HANDLE           hMeasurementMgr;
    TI_HANDLE           hTWD;
    TI_HANDLE           hMlmeSm;
    TI_HANDLE           hReport;
    TI_HANDLE           hOs;
    TI_HANDLE           hCcxMngr;
    TI_HANDLE           hApConn;
    TI_HANDLE           hCurrBss;
    TI_HANDLE           hQosMngr;
    TI_HANDLE           hPowerMgr;
    TI_HANDLE           hEvHandler;
    TI_HANDLE           hScr;
    TI_HANDLE           hStaCap;

    TI_UINT32           beaconSentCount;
    TI_UINT32           rxPacketsCount;
    TI_UINT32           txPacketsCount;
    TI_UINT16           txSessionCount;     /* Current Tx-Session index as configured to FW in last Join command. */

    EModulationType     chosenModulation;
    EModulationType     currentDataModulation;
    EDot11Mode          siteMgrOperationalMode;
    ERadioBand          radioBand;
    ERadioBand          prevRadioBand;
    
    TMacAddr            ibssBssid;
    TI_BOOL             bPostponedDisconnectInProgress;
    TI_BOOL             isAgingEnable;

    /* TX Power Adjust */
    TI_UINT32           siteMgrTxPowerCheckTime;
    TI_BOOL             siteMgrTxPowerEnabled;

    TBeaconFilterInitParams    beaconFilterParams; /*contains the desired state*/


    /*HW Request from Power Ctrl */
    TI_UINT32           DriverTestId;

    /* Wifi Simple Config */
    TIWLN_SIMPLE_CONFIG_MODE  siteMgrWSCCurrMode; /* indicates the current WiFi Simple Config mode */
    char                siteMgrWSCProbeReqParams[DOT11_WSC_PROBE_REQ_MAX_LENGTH]; /* Contains the params to be used in the ProbeReq - WSC IE */ 

    TI_UINT8            includeWSCinProbeReq;
} siteMgr_t;



siteEntry_t *findAndInsertSiteEntry(siteMgr_t       *pSiteMgr,
                                    TMacAddr    *bssid,
                                    ERadioBand     band);

siteEntry_t *findSiteEntry(siteMgr_t        *pSiteMgr,
                           TMacAddr     *bssid);

void removeSiteEntry(siteMgr_t *pSiteMgr, siteTablesParams_t *pCurrSiteTblParams,
                     siteEntry_t  *hashPtr);

TI_STATUS removeEldestSite(siteMgr_t *pSiteMgr);

TI_STATUS templates_buildAPBeaconTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TBssCapabilities *pBssCap);

TI_STATUS buildProbeReqTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TSsid *pSsid, ERadioBand radioBand);

TI_STATUS buildProbeRspTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate);

TI_STATUS buildNullTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate);

TI_STATUS buildArpRspTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TIpAddr staIp);

TI_STATUS buildDisconnTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate);

TI_STATUS buildPsPollTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate);

TI_STATUS buildQosNullDataTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TI_UINT8 userPriority);

void setDefaultProbeReqTemplate (TI_HANDLE	hSiteMgr);

#endif /* __SITE_MGR_H__ */
