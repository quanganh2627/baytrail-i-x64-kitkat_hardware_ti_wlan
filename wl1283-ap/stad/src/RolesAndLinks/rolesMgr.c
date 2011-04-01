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

/** \file rolesMgr.c
 *  \brief Roles Manager info
 *
 *  \see rolesMgr.h
 */


#define __FILE_ID__  FILE_ID_139
#include "rolesMgr.h"
#include "roleAP.h"
#include "osApi.h"
#include "report.h"
#include "802_11Defs.h"
#include "TWDriver.h"
#include "EvHandler.h"
#include "DrvMainModules.h"

#include "siteHash.h"


/* Constants */

/* Enumerations */

/* Typedefs */

/* Structures */
 
/* Internal functions prototypes */

/* Public functions implementation */





/** 
 * \fn     rolesMgr_create 
 * \brief  Create RolesMgr object
 * 
 * Allocate and clear the module's object
 */ 
TI_HANDLE rolesMgr_create(TI_HANDLE hOs)
{
    tRolesMgr   *pRolesMgr;

    if ((pRolesMgr = os_memoryAlloc(hOs, sizeof(tRolesMgr))) != NULL)
    {
        pRolesMgr->hOs = hOs;
    
        return pRolesMgr;
    }
    else /* Failed to allocate control block */
    {
        WLAN_OS_REPORT(("\n FATAL ERROR: rolesMgr_create(): Error allocating cb - aborting\n"));
        return NULL;
    }
}




/** 
 * \fn     rolesMgr_destroy 
 * \brief  Destroy RolesMgr object
 * 
 * Free the module's object memory
 */ 
TI_STATUS rolesMgr_destroy(TI_HANDLE hRolesMgr)
{
    tRolesMgr   *pRolesMgr;
    
    if (hRolesMgr != NULL)
    {
        pRolesMgr = (tRolesMgr *)hRolesMgr;

        /* Free pre-allocated control block */
        os_memoryFree(pRolesMgr->hOs, pRolesMgr, sizeof(tRolesMgr));
    }
    return TI_OK;
}



/** 
 * \fn     rolesMgr_init 
 * \brief  Init RolesMgr object
 * 
 * Init module's object and link its handles
 */ 
void rolesMgr_init (TStadHandlesList *pStadHandles)
{
    tRolesMgr *pRolesMgr = (tRolesMgr *)(pStadHandles->hRolesMgr);

    pRolesMgr->hOs           = pStadHandles->hOs;
    pRolesMgr->hReport       = pStadHandles->hReport;
    pRolesMgr->hEvHandler    = pStadHandles->hEvHandler;
    pRolesMgr->hTWD          = pStadHandles->hTWD;

    pRolesMgr->hSiteMgr      = pStadHandles->hSiteMgr;
    pRolesMgr->hSME          = pStadHandles->hSme;
    pRolesMgr->hScr          = pStadHandles->hSCR;

    pRolesMgr->hRoleAP       = pStadHandles->hRoleAP;
   
}



/** 
 * \fn     rolesMgr_SetDefaults 
 * \brief  set RolesMgr object default values
 * 
 * Set module's object default values 
 */ 
TI_STATUS rolesMgr_SetDefaults (TI_HANDLE hRolesMgr, TRolesMgrInitParams *pInitParams)
{
    tRolesMgr *pRolesMgr = (tRolesMgr *)hRolesMgr;
   
    pRolesMgr->uActiveRoleType = pInitParams->uActiveRoleType;; 

    return TI_OK;
}


/** 
 * \fn     rolesMgr_setParam 
 * \brief  RolesMgr object setParam API 
 * 
 * Module's object set param API 
 */ 
TI_STATUS rolesMgr_setParam(TI_HANDLE hRolesMgr, paramInfo_t *pParam)
{
    tRolesMgr *pRolesMgr = (tRolesMgr *)hRolesMgr;
    TI_STATUS   status = TI_OK;

    if (pParam == NULL)
    {
        TRACE0(pRolesMgr->hReport, REPORT_SEVERITY_ERROR , " rolesMgr_setParam(): pParam is NULL!\n");
        return TI_NOK;
    }
	
    TRACE1(pRolesMgr->hReport,REPORT_SEVERITY_INFORMATION , "rolesMgr_setParam() %X \n", pParam->paramType);    

    switch (pParam->paramType)
    {
	    case 0:
        {
  
        }
        break;

        default:
            TRACE1(pRolesMgr->hReport, REPORT_SEVERITY_ERROR, "rolesMgr_setParam bad param=  %X\n", pParam->paramType);
            break;
    }

    return status;
}



/** 
 * \fn     rolesMgr_getParam 
 * \brief  RolesMgr object getParam API 
 * 
 * Module's object set param API 
 */ 
TI_STATUS rolesMgr_getParam(TI_HANDLE hRolesMgr, paramInfo_t *pParam)
{
    return TI_NOK;
}



/** 
 * \fn     rolesMgr_start 
 * \brief  RolesMgr start command API
 * 
 * Start RolesMgr object - send configuration & start cmd to FW
 */ 
TI_STATUS rolesMgr_start(TI_HANDLE hRolesMgr)
{
    tRolesMgr *pRolesMgr = (tRolesMgr *)hRolesMgr;
    
    /* set SCR group according to connection mode - to be removed later [liorC] */
    scr_setGroup (pRolesMgr->hScr, SCR_GID_DRV_SCAN);

    return roleAP_start(pRolesMgr->hRoleAP, 0);
}
