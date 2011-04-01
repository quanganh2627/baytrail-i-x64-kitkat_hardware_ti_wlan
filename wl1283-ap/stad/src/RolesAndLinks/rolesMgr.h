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

/* \file   rolesMgr.h 
 *  \brief  RolesMgr API definition                                  
 * 
 *  RoleAP module provides:
 * 
 * 	- Save Bss confguration received by Hostapd application.
 *  - Start the AP Role in FW by Sending Configuration & start/stop commands 
 *  \see    rolesMgr.c
 */

#ifndef _ROLES_MGR_H_
#define _ROLES_MGR_H_


#include "paramOut.h"

/* Constants */

/* Enumerations */

/* Structures */



/* rolesMgr object */
typedef struct
{

    TI_HANDLE   hReport;
    TI_HANDLE   hOs;
    TI_HANDLE   hEvHandler;
    TI_HANDLE   hTWD;
    TI_HANDLE   hSiteMgr;
    TI_HANDLE   hSME;
    TI_HANDLE   hScr;
    TI_HANDLE   hRoleAP;

    TI_UINT8    uActiveRoleType;     
} tRolesMgr;



/** 
 * \fn     rolesMgr_create 
 * \brief  Create RolesMgr object
 * 
 * Allocate and clear the module's object
 * 
 * \note
 * \param	hOs    - Handle to OS context
 * \return 	Handle of the allocated object, NULL if allocation failed 
 * \sa     	drvMain_create
 */ 
TI_HANDLE rolesMgr_create(TI_HANDLE hOs);



/** 
 * \fn     rolesMgr_destroy 
 * \brief  Destroy RolesMgr object
 * 
 * Free the module's object memory
 * 
 * \note
 * \param	hRolesMgr - Handle to RolesMgr object
 * \return 	TI_OK
 * \sa     	drvMain_destroy
 */ 
TI_STATUS rolesMgr_destroy(TI_HANDLE hRolesMgr);


/** 
 * \fn     rolesMgr_init 
 * \brief  Init RolesMgr object
 * 
 * Init module's object and link its handles
 * 
 * \note
 * \param	pStadHandles - Handle to StadHandles
 * \return 	Void
 * \sa     	drvMain_Init
 */ 
void rolesMgr_init (TStadHandlesList *pStadHandles);


/** 
 * \fn     rolesMgr_SetDefaults 
 * \brief  set RolesMgr object default values
 * 
 * Set module's object default values 
 * 
 * \note
 * \param	hRolesMgr - Handle to RolesMgr object
 * \param	pInitParams - Handle to object's init params structure
 * 
 * \return 	TI_OK
 * \sa     	drvMain_SetDefaults
 */ 
TI_STATUS rolesMgr_SetDefaults (TI_HANDLE hRolesMgr, TRolesMgrInitParams *pInitParams);


/** 
 * \fn     rolesMgr_setParam 
 * \brief  RolesMgr object setParam API 
 * 
 * Module's object set param API 
 * 
 * \note
 * \param	hRolesMgr - Handle to RolesMgr object
 * \param	pParam - Handle to generic paramInfo structure
 * 
 * \return 	TI_OK if success , otherwise - TI_NOK
 * \sa     	cmdDispathcer CB
 */ 
TI_STATUS rolesMgr_setParam(TI_HANDLE hRolesMgr, paramInfo_t *pParam);



/** 
 * \fn     rolesMgr_getParam 
 * \brief  RolesMgr object getParam API 
 * 
 * Module's object set param API 
 * 
 * \note
 * \param	hRolesMgr - Handle to RolesMgr object
 * \param	pParam - Handle to generic paramInfo structure to be filled
 * 
 * \return 	TI_OK if success , otherwise - TI_NOK
 * \sa     	cmdDispathcer CB
 */ 
TI_STATUS rolesMgr_getParam(TI_HANDLE hRolesMgr, paramInfo_t *pParam);



/** 
 * \fn     rolesMgr_start 
 * \brief  RolesMgr start command API
 * 
 * Start RolesMgr object - send configuration & start cmd to FW
 * 
 * \note
 * \param	hRolesMgr - Handle to RolesMgr object
 * 
 * \return 	TI_OK if success , otherwise - TI_NOK
 * \sa     	drvMain_Sm
 */ 
TI_STATUS rolesMgr_start(TI_HANDLE hRolesMgr);


#endif /*  _ROLES_MGR_API_H_*/

