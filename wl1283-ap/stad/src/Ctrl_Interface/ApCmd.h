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

 /* \file   ApCmd.h 
 *  \brief  ApCmd API definition                                  
 * 
 *  ApCmd module provides:
 * 
 * 	- translate and dispatch of hostapd commands.
 *  - 
 *  \see    ApCmd.c
 */


#ifndef _AP_CMD_H_
#define _AP_CMD_H_

#include "CmdHndlr.h"

/** 
 * \fn     apCmd_Execute 
 * \brief  execute AP external command
 * 
 * translate and dispatch hostapd commands
 * 
 * \note
 * \param	handle - Handle to ApCmd module , cmdObj
 * \return 	success/fail to process the cmd
 * \sa     	cmdHndlr_HandleCommands
 */ 

int apCmd_Execute(TI_HANDLE hApCmd, TConfigCommand *cmdObj);
/** 
 * \fn     apCmd_init 
 * \brief  Init apCmd_init object
 * 
 * Init module's object and link its handles
 * 
 * \note
 * \param	pStadHandles - Handle to StadHandles
 * \return 	Void
 * \sa     	drvMain_Init
 */ 

void apCmd_init(TStadHandlesList *pStadHandles);
/** 
 * \fn     apCmd_create 
 * \brief  Create apCmd_create object
 * 
 * Allocate and clear the module's object
 * 
 * \note
 * \param	hOs    - Handle to OS context
 * \return 	Handle of the allocated object, NULL if allocation failed 
 * \sa     	drvMain_create
 */ 

TI_HANDLE apCmd_create(TI_HANDLE hOs);

/** 
 * \fn     apCmd_destroy 
 * \brief  Destroy apCmd_destroy object
 * 
 * Free the module's object memory
 * 
 * \note
 * \param	hApCmd - Handle to apCmd_destroy object
 * \return 	TI_OK
 * \sa     	drvMain_destroy
 */ 

TI_STATUS apCmd_destroy(TI_HANDLE hApCmd);

/** 
 * \fn     apCmd_ServiceCompleteCB 
 * \brief  calling by driver module to end pending commands
 * 
 * * \note
 * \param	hApCmd - Handle to apCmd
 *          status - command status
 * 
 * * \return 	TI_OK
 * \sa     	RoleAp_ApSetParam *
 **/ 

int apCmd_ServiceCompleteCB (TI_HANDLE hApCmd, int status, void *buffer);



#endif /* _AP_CMD_H_ */
