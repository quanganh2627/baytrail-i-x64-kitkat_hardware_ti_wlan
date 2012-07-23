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

/****************************************************************************
 *                                                                          *
 *   MODULE:  	    		                                                *
 *   PURPOSE:                                                             	*
 *                                                                          *
 ****************************************************************************/
#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_15
#include "osApi.h"
#include "paramOut.h"
#include "fsm.h"
#include "report.h"
#include "802_11Defs.h"
#include "siteHash.h"
#include "mlmeSm.h"
#include "mlmeApi.h"


/* Ccx transmit Power element */

#define CELL_POWER_LIMIT_IE_SIZE    8 
typedef struct 
{
	dot11_eleHdr_t	hdr;
	TI_UINT8		    oui[4];
    TI_UINT8           power;
}ccxTxPower_t;

TI_STATUS Ccx_ParseClientTP(TI_HANDLE hMlme, siteEntry_t *pSite,TI_INT8 *CellPowerlimit 
                            ,TI_HANDLE pIeBuffer, TI_UINT16 length)
{
    mlme_t *pMlme = (mlme_t*) hMlme;
    TI_UINT8 Ccx_OUI[] = CCX_OUI;  
    TI_UINT8 *pDesiredIe;
    ccxTxPower_t *pCcxTxPowerIE;
    
    /*If the power restriction element is present in this beacon or probe response 
    the ccx must not effect the Tx power.*/
    if (mlmeParser_ParseIeBuffer(hMlme, pIeBuffer, length, POWER_CONSTRAINT_IE_ID, &pDesiredIe, NULL, 0))
        return TI_NOK;
    
    /* Check for Ccx transmit Power element. */
    if (!mlmeParser_ParseIeBuffer(hMlme, pIeBuffer, length, CELL_POWER_IE, &pDesiredIe, NULL, 0))
        return TI_NOK;
    
    pCcxTxPowerIE = (ccxTxPower_t*)pDesiredIe;
    
    /* Verify that this is Ccx Info elemnt*/
    if (!os_memoryCompare (pMlme->hOs, pCcxTxPowerIE->oui, Ccx_OUI, 3))
    {
        /*Extract the Power Level in dbm from the IE*/
        *CellPowerlimit = pCcxTxPowerIE->power;
        return TI_OK;
    }
    return TI_NOK;
}
#endif /*CCX_MODULE_INCLUDED*/



