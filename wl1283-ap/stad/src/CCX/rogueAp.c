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
/** \file rogueAp.c
 * \brief Rogue Ap handler as defined in CCX spec (Cisco Compatible Extensions for WLAN Devices).
 *
 * \see rogueAp.h
*/


/****************************************************************************
 *                                                                          *
 *   MODULE:  Rogue AP handler                                              *
 *   PURPOSE:                                                               *
 *   This module is responsible to handle Rogue AP as defined in CCX
 *		(Cisco Compatible Extensions for WLAN Devices) spec.
 *		On any LEAP authentication failure the AP is marked as Rogue
 *		with the Rogue reason which can be: invalid type, timeout, 
 * 		challenge to/from AP failed.
 * 		When successful LEAP authentication is established, the Rogue APs	
 *		are reported and the list is reset.
 *                                             
 *                                                                          *
 ****************************************************************************/
#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_14
#include "osApi.h"
#include "report.h"
#include "paramOut.h"
#include "rogueAp.h"
#include "ccxMngr.h"


#define ROGUE_AP_LEN            48
#define ROGUE_AP_FUNCTION_CODE  0x8E
#define DUMMY_MAC_ADDRESS       {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

/* The Rogue AP message body as defined in CCX V1 spec */
typedef struct 
{
    TI_UINT16          failureReason;
    TMacAddr           rogueApMacAddress;
    TI_UINT8           rogueApName[ROGUE_AP_NAME_LEN];
} rogue_AP_msg_body_t;

/* The Rogue AP message body+IAPP header as defined in CCX V1 spec */
typedef struct
{
	IAPP_msg_header_t   	IAPP_msg_header;
	rogue_AP_msg_body_t		IAPP_Rogue_AP_msg_body; 
} rogue_AP_msg_t;
												  
/* Functions */
TI_STATUS rogueAp_buildRogueApMsg(rogueAp_t	*pRogueAp, TI_UINT16 index, rogue_AP_msg_t *rogueApMsg, TI_UINT32 *length);
TI_STATUS rogueAp_resetRogueAp(rogueAp_t* pRogueAp);
TI_UINT8 rogueAp_findRogueAp(rogueAp_t* pRogueAp, TMacAddr macAddress, TI_BOOL *isRogue);



/**
*
* rogueAp_create
*
* \b Description: 
*
* Create the Rogue AP context.
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
rogueAp_t* rogueAp_create(TI_HANDLE hOs)
{
	rogueAp_t 		*pRogueAp;

	/* allocate context memory */
	pRogueAp = (rogueAp_t*)os_memoryAlloc(hOs, sizeof(rogueAp_t));
	if (pRogueAp == NULL)
	{
		return NULL;
	}

	os_memoryZero(hOs, pRogueAp, sizeof(rogueAp_t));

	pRogueAp->hOs = hOs;

	return pRogueAp;
}

/**
*
* rogueAp_unload
*
* \b Description: 
*
* Unload Rogue AP module from memory
*
* \b ARGS:
*
*  I   - hRogueAp - Rogue AP context  \n
*
* \b RETURNS:
*
*  TI_OK if successful, TI_NOK otherwise.
*
* \sa rogueAp_create
*/
TI_STATUS rogueAp_unload(TI_HANDLE hRogueAp)
{
	rogueAp_t		*pRogueAp;

	if (hRogueAp == NULL)
	{
		return TI_NOK;
	}
	
	pRogueAp = (rogueAp_t*)hRogueAp;
	
	os_memoryFree(pRogueAp->hOs, pRogueAp, sizeof(rogueAp_t));

	return TI_OK;
}

/**
*
* rogueAp_config
*
* \b Description: 
*
* Configure the Rogue AP module.
*
* \b ARGS:
*
*  I   - hRogueAp - Rogue AP context  \n
*  I   - hReport - Report context  \n
*  I   - hOs - OS context  \n
*  I   - hCcxMngr - CCX manager context  \n
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure.
*
* \sa 
*/
TI_STATUS rogueAp_config(TI_HANDLE hRogueAp,
					  TI_HANDLE hReport,
					  TI_HANDLE hOs,
					  TI_HANDLE hCcxMngr)
{
	rogueAp_t		*pRogueAp;

	if (hRogueAp == NULL)
	{
		return TI_NOK;
	}
	
	pRogueAp = (rogueAp_t*)hRogueAp;

	pRogueAp->hReport 	= hReport;
	pRogueAp->hCcxMngr	= hCcxMngr;

	return TI_OK;
}



/******** Rogue AP functions *********/

/**
*
* rogueAp_resetRogueAp
*
* \b Description: 
*
* Resets the rogue AP table.
* 
*
* \b ARGS:
*
* I - pRogueAp - pointer to the rogueAp object
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise. 
*
* \sa 
*
*/
TI_STATUS rogueAp_resetRogueAp(rogueAp_t* pRogueAp)
{

	TI_UINT8    index;

TRACE0(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "resetRogueAp \n");
	pRogueAp->rogueApHead = 0;
    pRogueAp->rogueApTail = 0;

	for (index=0; index<MAX_ROGUE_AP_NUMBER; index++)
    {
		pRogueAp->rogueApTable[index].isRogue = TI_FALSE;
	}

    return(TI_OK);
}

/**
*
* rogueAp_addRogueAp
*
* \b Description: 
*
* Adds a rogue AP entry to the table.
* 
*
* \b ARGS:
*
* I - pRogueAp - pointer to the rogueAp object
* I - pRogueApDetected - rogue AP entry
* 
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise. 
*
* \sa 
*
*/
TI_STATUS rogueAp_addRogueAp(TI_HANDLE hRogueAp, OS_CCX_ROGUE_AP_DETECTED *pRogueApDetected)
{
    TI_UINT8           index;
    TI_BOOL            isRogue;
	rogueAp_t		*pRogueAp;

	if (hRogueAp == NULL)
	{
		return TI_NOK;
	}
	
	pRogueAp = (rogueAp_t*)hRogueAp;


TRACE1(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "rogueAp_addRogueAp entry , failureReason = %d\n", pRogueApDetected->FailureReason);


    index = rogueAp_findRogueAp(pRogueAp, (TI_UINT8 *)pRogueApDetected->RogueAPMacAddress, &isRogue); 

    if (isRogue)
    {   /* This AP is already rogue */
        TRACE0(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "addRogueAp is already Rogue\n");
        return TI_NOK;
    }
    if (pRogueAp->rogueApTail >= MAX_ROGUE_AP_NUMBER)
    {
        TRACE1(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "addRogueAp bad tail index=%d\n", pRogueAp->rogueApTail);
        return TI_NOK;
    }
	pRogueAp->rogueApTable[pRogueAp->rogueApTail].failureReason = (EAuthStatus)pRogueApDetected->FailureReason;
	pRogueAp->rogueApTable[pRogueAp->rogueApTail].isRogue = TI_TRUE;
    MAC_COPY (pRogueAp->rogueApTable[pRogueAp->rogueApTail].rogueApMacAddress, 
              pRogueApDetected->RogueAPMacAddress);
	os_memoryCopy(pRogueAp->hOs, 
                  pRogueAp->rogueApTable[pRogueAp->rogueApTail].rogueApName, 
                  (void*)pRogueApDetected->RogueAPName, 
                  ROGUE_AP_NAME_LEN);
    index = pRogueAp->rogueApTail;
    pRogueAp->rogueApTail = (pRogueAp->rogueApTail+1)%MAX_ROGUE_AP_NUMBER;
    if (pRogueAp->rogueApHead==pRogueAp->rogueApTail)
    {
        pRogueAp->rogueApHead = (pRogueAp->rogueApHead+1)%MAX_ROGUE_AP_NUMBER;
    }

TRACE10(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "addRogueAp,index=%d, isRogue=%d, MAC= 0x%x,  0x%x,0x%x,0x%x,0x%x,0x%x\nrogueApHead=%d, rogueApTail=%d\n", index, pRogueAp->rogueApTable[index].isRogue, pRogueApDetected->RogueAPMacAddress[0], pRogueApDetected->RogueAPMacAddress[1], pRogueApDetected->RogueAPMacAddress[2], pRogueApDetected->RogueAPMacAddress[3], pRogueApDetected->RogueAPMacAddress[4], pRogueApDetected->RogueAPMacAddress[5], pRogueAp->rogueApHead, pRogueAp->rogueApTail);
    return(TI_OK);
}

/**
*
* rogueAp_findRogueAp
*
* \b Description: 
*
* Finds a rogue AP entry according to MAC address.
* 
*
* \b ARGS:
*
* I - pRogueAp - pointer to the rogueAp object
* I - macAddress - mac address of the rogued AP 
* O - isRogue: TI_TRUE if Rogue, TI_FALSE otherwise 
*  
* \b RETURNS:
*
* index: the index in the table of the rogue AP 
*
* \sa 
*
*/
TI_UINT8 rogueAp_findRogueAp(rogueAp_t* pRogueAp, TMacAddr macAddress, TI_BOOL *isRogue)
{
    TI_UINT8  index;
	
    TRACE6(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "findRogueAp, macAddress= 0x%0x, 0x%0x, 0x%0x, 0x%0x,0x%0x, 0x%0x \n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
	
    *isRogue = TI_FALSE;
    for (index=pRogueAp->rogueApHead; index!=pRogueAp->rogueApTail; index=(index+1)%MAX_ROGUE_AP_NUMBER) 
    {
      TRACE7(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "findRogueAp, index=%d,rogueApMac= 0x%0x, 0x%0x, 0x%0x, 0x%0x,0x%0x, 0x%0x \n", index, pRogueAp->rogueApTable[index].rogueApMacAddress[0], pRogueAp->rogueApTable[index].rogueApMacAddress[1], pRogueAp->rogueApTable[index].rogueApMacAddress[2], pRogueAp->rogueApTable[index].rogueApMacAddress[3], pRogueAp->rogueApTable[index].rogueApMacAddress[4], pRogueAp->rogueApTable[index].rogueApMacAddress[5]                           );
        if (MAC_EQUAL(pRogueAp->rogueApTable[index].rogueApMacAddress, macAddress)) 
        {
            *isRogue = pRogueAp->rogueApTable[index].isRogue;
            TRACE1(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "findRogueAp IS Rogue, index=%d\n", index);
            break;
        }
    }
    if (*isRogue==TI_FALSE)
    {
        TRACE1(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "findRogueAp NOT Rogue, index=%d\n", index);
    }
    return(index);
}



/**
*
* rogueAp_buildRogueApMsg
*
* \b Description: 
*
* Builds one rogue AP message.
* 
*
* \b ARGS:
*
* I - pRogueAp - pointer to the rogueAp object
* I - index - index of the rogued AP entry in the table
* I - rogueApMsg: a pointer to the rogue AP message 
* I - length: the length of the rogue AP message  
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise. 
*
* \sa 
*
*/
TI_STATUS rogueAp_buildRogueApMsg(rogueAp_t	*pRogueAp, TI_UINT16 index, rogue_AP_msg_t *rogueApMsg, TI_UINT32 *length)
{
    rogue_AP_msg_t  rogue_AP_msg={{IAPP_PACKET_SNAP, 0, IAPP_ROGUE_AP, 
        ROGUE_AP_FUNCTION_CODE, DUMMY_MAC_ADDRESS, DUMMY_MAC_ADDRESS},
    {0x00, DUMMY_MAC_ADDRESS, ""}};

    rogue_AP_msg.IAPP_msg_header.length = HTOWLANS(ROGUE_AP_LEN);
    if ((pRogueAp==NULL) || (index >= MAX_ROGUE_AP_NUMBER) || (rogueApMsg==NULL)|| (length==NULL) )
    {
        return TI_NOK;
    }

    *length=0;

    TRACE4(pRogueAp->hReport, REPORT_SEVERITY_INFORMATION, "buildRogueApMsg, index=%d, head=%d, tail=%d, rogueApMsg=%p \n", index, pRogueAp->rogueApHead, pRogueAp->rogueApTail, rogueApMsg);
	/* Check index validity */
    if (pRogueAp->rogueApTail>=pRogueAp->rogueApHead)
    {   /* There was no queue overflow, make sure the index in between the Head and Tail */
        if ((index>=pRogueAp->rogueApTail) || (index<pRogueAp->rogueApHead))
        {
            return TI_OK;
        }
    }
    else
    {   /* There was a queue overflow, make sure the index in between the Tail and Head */
        if ((index>=pRogueAp->rogueApTail) && (index<pRogueAp->rogueApHead))
        {
            return TI_OK;
        }
    }
	/* Rogue AP failure reason */
    rogue_AP_msg.IAPP_Rogue_AP_msg_body.failureReason = (TI_UINT16)(pRogueAp->rogueApTable[index].failureReason);
    rogue_AP_msg.IAPP_Rogue_AP_msg_body.failureReason = HTOWLANS(rogue_AP_msg.IAPP_Rogue_AP_msg_body.failureReason); 
    /* Rogue AP MAC */
	MAC_COPY (rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress, pRogueAp->rogueApTable[index].rogueApMacAddress);
    /* Rogue AP name */
	os_memoryCopy(pRogueAp->hOs, rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApName, pRogueAp->rogueApTable[index].rogueApName, ROGUE_AP_NAME_LEN);
    *length = ROGUE_AP_LEN;
    if (rogueApMsg!=NULL) 
        os_memoryCopy(pRogueAp->hOs, rogueApMsg, &rogue_AP_msg, ROGUE_AP_LEN);
    else
        *length = 0;

    TRACE31(pRogueAp->hReport, REPORT_SEVERITY_INFORMATION, "buildRogueApMsg, length= 0x%x, SNAP= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, \n length = 0x%x, type =0x%x, code=0x%x, \n DA MAC=0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x,SA MAC = 0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x, \n fail reason = 0x%x, Rogue AP MAC=0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x\n", *length, 						 rogue_AP_msg.IAPP_msg_header.snapHeader[0], rogue_AP_msg.IAPP_msg_header.snapHeader[1], rogue_AP_msg.IAPP_msg_header.snapHeader[2], rogue_AP_msg.IAPP_msg_header.snapHeader[3], rogue_AP_msg.IAPP_msg_header.snapHeader[4], rogue_AP_msg.IAPP_msg_header.snapHeader[5], rogue_AP_msg.IAPP_msg_header.snapHeader[6], rogue_AP_msg.IAPP_msg_header.snapHeader[7], 						 rogue_AP_msg.IAPP_msg_header.length, rogue_AP_msg.IAPP_msg_header.messageType, rogue_AP_msg.IAPP_msg_header.funcCode, 						 rogue_AP_msg.IAPP_msg_header.destinationMacAddress[0], rogue_AP_msg.IAPP_msg_header.destinationMacAddress[1], rogue_AP_msg.IAPP_msg_header.destinationMacAddress[2], rogue_AP_msg.IAPP_msg_header.destinationMacAddress[3], rogue_AP_msg.IAPP_msg_header.destinationMacAddress[4], rogue_AP_msg.IAPP_msg_header.destinationMacAddress[5], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[0], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[1], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[2], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[3], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[4], rogue_AP_msg.IAPP_msg_header.sourceMacAddress[5], 						 rogue_AP_msg.IAPP_Rogue_AP_msg_body.failureReason, rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[0], rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[1], rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[2], rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[3], rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[4], rogue_AP_msg.IAPP_Rogue_AP_msg_body.rogueApMacAddress[5]                         );
    return TI_OK;
}

/**
*
* rogueAp_report
*
* \b Description: 
*
* Builds and sends a rogue AP IAPP packet.
* 
*
* \b ARGS:
*
* I - hRogueAp - pointer to the rogueAp object
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise. 
*
* \sa 
*
*/
TI_STATUS rogueAp_report(TI_HANDLE hRogueAp, TMacAddr rogueApMac)
{
    TI_UINT32       	length=0;
    rogue_AP_msg_t  rogueApbufMsg;
	TI_UINT16       	index;
	TI_STATUS    	status;
	rogueAp_t	 	*pRogueAp;

	if (hRogueAp == NULL)
	{
		return TI_NOK;
	}
	
	pRogueAp = (rogueAp_t*)hRogueAp;


TRACE0(pRogueAp->hReport , REPORT_SEVERITY_INFORMATION, "rogueAp_report \n");


	for (index=0; index<MAX_ROGUE_AP_NUMBER; index++)
    {
		if (!pRogueAp->rogueApTable[index].isRogue)
		{   /* skip to next entry */ 
			continue;
		}
        if (!os_memoryCompare(pRogueAp->hOs, (void*)pRogueAp->rogueApTable[index].rogueApMacAddress, rogueApMac, 6))
        {    /* skip on Current Connected AP */ 
			continue;
        }

		status = rogueAp_buildRogueApMsg(pRogueAp, index, &rogueApbufMsg, &length); 
        if (status!=TI_OK)
        {
            return status;
        }
        if (length)
        {
            status = ccxMngr_sendIAPPPacket(pRogueAp->hCcxMngr, &rogueApbufMsg.IAPP_msg_header, (TI_UINT8*)&rogueApbufMsg.IAPP_Rogue_AP_msg_body, length);
			if (status != TI_OK)
			{
				return status;
			}
        }
    }

    status = rogueAp_resetRogueAp(pRogueAp);

    return status;
}

#ifdef TI_DBG
/* For debug */
void printRogueApTable(TI_HANDLE hRogueAp)
{
    TI_UINT32      index;
	rogueAp_t	 *pRogueAp;

	if (hRogueAp == NULL)
	{
		return;
	}

	pRogueAp = (rogueAp_t*)hRogueAp;

    TRACE0(pRogueAp->hReport, REPORT_SEVERITY_INFORMATION, "printRogueApTable\n");

    for (index=0; index<MAX_ROGUE_AP_NUMBER; index++)
    {
        if (!pRogueAp->rogueApTable[index].isRogue)
        {   /* skip to next entry */ 
            continue;
        }
        TRACE7(pRogueAp->hReport, REPORT_SEVERITY_INFORMATION, "fail reason = 0x%x, Rogue AP MAC=0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x\n", pRogueAp->rogueApTable[index].failureReason, pRogueAp->rogueApTable[index].rogueApMacAddress[0], pRogueAp->rogueApTable[index].rogueApMacAddress[1], pRogueAp->rogueApTable[index].rogueApMacAddress[2], pRogueAp->rogueApTable[index].rogueApMacAddress[3], pRogueAp->rogueApTable[index].rogueApMacAddress[4], pRogueAp->rogueApTable[index].rogueApMacAddress[5]);

    }

}
#endif
#endif /*CCX_MODULE_INCLUDED*/


