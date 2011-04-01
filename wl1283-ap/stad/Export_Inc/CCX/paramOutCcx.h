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

/*--------------------------------------------------------------------------*/
/* Module:		paramOutCcx.h */
/**/
/* Purpose:		*/
/**/
/*--------------------------------------------------------------------------*/
#ifndef __PARAMOUTCCX_H__
#define __PARAMOUTCCX_H__

#ifdef CCX_MODULE_INCLUDED

#include "osDot11.h"
#include "osDot11Ccx.h"

#define   CCX_PARAM_FIELDS \
	TI_BOOL                    rsnCcxSwEncFlag; \
	TI_BOOL                    rsnCcxMicFieldFlag; \
	OS_CCX_NETWORK_EAP         networkEap; \
	OS_CCX_AUTH_SUCCESS        authSuccess; \
	OS_CCX_ROGUE_AP_DETECTED   rogueApDetected; \
	TMacAddr                   reportRogueAp; \
	TI_UINT32                  ccxConfiguration; \
	OS_CCX_CCKM_REQUEST        ccxCckmRequest; \
 	OS_CCX_CCKM_RESULT         ccxCckmResult; \
	TI_BOOL                    ccxCckmExists; \
	TI_BOOL                    ccxEnabled; \
	TI_UINT8				   ccxVersion;


#endif /*CCX_MODULE_INCLUDED*/

#endif /*__PARAMOUTCCX_H__*/
