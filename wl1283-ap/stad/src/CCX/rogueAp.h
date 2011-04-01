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
/** \file rogueAp.h
 * \brief Rogue Ap IF as defined in CCX spec (Cisco Compatible Extensions for WLAN Devices).
 *
 * \see rogueAp.c, ccxMngr.c
*/


/****************************************************************************
 *                                                                          *
 *   MODULE:   Rogue AP                                                     *
 *   PURPOSE:  Rogue AP API                                                 *
 *                                                                          *
 ****************************************************************************/

#ifndef __INCLUDE_ROGUE_AP_H
#define __INCLUDE_ROGUE_AP_H
#ifdef CCX_MODULE_INCLUDED

/* Rogue AP types */

#define ROGUE_AP_NAME_LEN 16
#define MAX_ROGUE_AP_NUMBER  33

/* Rogue AP DB */
typedef struct
{
    EAuthStatus     failureReason;
    TI_BOOL         isRogue;
    TMacAddr        rogueApMacAddress;
    TI_UINT8        rogueApName[ROGUE_AP_NAME_LEN];
} rogueApEntry_t;

    
typedef struct _rogueAp_t
{
    /* roague AP list */
	rogueApEntry_t          rogueApTable[MAX_ROGUE_AP_NUMBER];
    TI_UINT8                   rogueApHead;
    TI_UINT8                   rogueApTail;

	/* Handles to other objects */
	TI_HANDLE 				hReport;
	TI_HANDLE 				hOs;
	TI_HANDLE				hCcxMngr;

} rogueAp_t;

TI_STATUS rogueAp_unload(TI_HANDLE hRogueAp);

rogueAp_t* rogueAp_create(TI_HANDLE hOs);

TI_STATUS rogueAp_config(TI_HANDLE hRogueAp,
					  TI_HANDLE hReport,
					  TI_HANDLE hOs,
					  TI_HANDLE hCcxMngr);

TI_STATUS rogueAp_addRogueAp(TI_HANDLE hRogueAp, OS_CCX_ROGUE_AP_DETECTED *pRogueApDetected);

TI_STATUS rogueAp_report(TI_HANDLE hRogueAp, TMacAddr rogueApMac);

#endif/* CCX_MODULE_INCLUDED */
#endif /* __INCLUDE_ROGUE_AP_H */
