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

 
/** \file  FWDebug.c 
 * 
 *  \see   FWDebug.h 
 */

#define __FILE_ID__  FILE_ID_103
#include "tidef.h"
#include "fwDebug_api.h"
#include "osApi.h"
#include "report.h"
#include "BusDrv.h"
#include "TwIf.h"



#define DMA_SIZE_BUF 256

typedef struct 
{
    TI_HANDLE       hOs;
    TI_HANDLE       hReport;
	TI_HANDLE		hTwif;

	TFwDubCallback	fCb;
	TI_HANDLE hCb;

	TI_UINT8*		pReadBuf;
        
	TTxnStruct		tTxn;
	TI_UINT8*		pDMABuf;

}TFwDebug;

/* Local functions */
static void     fwDbg_WriteAddrCb   (TI_HANDLE hFwDebug,TTxnStruct* pTxn);
static void     fwDbg_ReadAddrCb    (TI_HANDLE hFwDebug,TTxnStruct* pTxn);



/*
 * \brief	Create the FW Debug module
 * 
 * \param  hOs  - Handle to OS
 * \return The created object
 * 
 * \par Description
 * This function will allocate memory to FW Debug module.
 * 
 * \sa 
 */
TI_HANDLE fwDbg_Create (TI_HANDLE hOs)
{
	TFwDebug *pFwDebug = (TFwDebug *)os_memoryAlloc(hOs,sizeof(TFwDebug));

    if (pFwDebug == NULL)
    {
        WLAN_OS_REPORT (("FATAL ERROR: fwDbg_Create(): Error Creating TFwDebug - Aborting\n"));
        return NULL;
    }

    /* reset module object */
    os_memoryZero (hOs, pFwDebug, sizeof (TFwDebug));
    pFwDebug->hOs = hOs;

    return pFwDebug;
}


/*
 * \brief	Initialize the module
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \param  hReport - Handle to report
 * \param  hTwif - Handle to TWIF
 * \return none
 * 
 * \par Description
 * 
 * 
 * \sa 
 */
void fwDbg_Init (TI_HANDLE hFwDebug,
				 TI_HANDLE hReport,
				 TI_HANDLE hTwif)
{
	TFwDebug* pFwDebug = (TFwDebug*)hFwDebug;
	pFwDebug->hReport  = hReport;
	pFwDebug->hTwif	   = hTwif;

	/* Allocate DMA memory for read write transact */
	pFwDebug->pDMABuf = (TI_UINT8*)os_memoryAlloc(pFwDebug->hOs,DMA_SIZE_BUF);
}


/*
 * \brief	Destroy the object
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \return none
 * 
 * \par Description
 * Deallocate the object memory
 * 
 * \sa 
 */
void fwDbg_Destroy (TI_HANDLE hFwDebug)
{
	TFwDebug* pFwDebug = (TFwDebug*)hFwDebug;

	if (pFwDebug)
	{
		if (pFwDebug->pDMABuf)
    	{
    		os_memoryFree(pFwDebug->hOs,pFwDebug->pDMABuf,DMA_SIZE_BUF);
    	}
        os_memoryFree(pFwDebug->hOs,pFwDebug,sizeof(pFwDebug));
	}
}


/*
 * \brief	Write Address to FW
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \param  Address - Absolute HW address
 * \param  Length - Length in byte to write
 * \param  Buffer - Buffer to copy to FW
 * \param  fCb - CB function
 * \param  hCb - CB Handle
 * \return none
 * 
 * \par Description
 * Write buffer to HW must receive length in byte max size 256 bytes
 * address must be absolute HW address.
 * 
 * \sa 
 */
TI_STATUS fwDbg_WriteAddr (TI_HANDLE hFwDebug,
                           TI_UINT32 Address,
                           TI_UINT32 Length,
                           TI_UINT8* Buffer,
                           TFwDubCallback fCb,
                           TI_HANDLE hCb)
{
	TI_STATUS rc;
	TTxnStruct *pTxn;
	TFwDebug* pFwDebug = (TFwDebug*)hFwDebug;

	pTxn = &pFwDebug->tTxn;

	/* check if length is large than default threshold */
	if (Length > DMA_SIZE_BUF)
    {
TRACE1(pFwDebug->hOs, REPORT_SEVERITY_ERROR, "fwDbg_WriteAddr : Buffer Length too large -- %d",Length);
		return TXN_STATUS_ERROR;
    }

	pFwDebug->fCb = fCb;
	pFwDebug->hCb = hCb;
	/* copy the given buffer to DMA buffer */
	os_memoryCopy(pFwDebug->hOs,pFwDebug->pDMABuf,Buffer,Length);
	/* Build the command TxnStruct */
    TXN_PARAM_SET(pTxn, TXN_LOW_PRIORITY, TXN_FUNC_ID_WLAN, TXN_DIRECTION_WRITE, TXN_INC_ADDR)
	/* Applying a CB in case of an async read */
    BUILD_TTxnStruct(pTxn, Address, pFwDebug->pDMABuf, Length,(TTxnDoneCb)fwDbg_WriteAddrCb, pFwDebug)
	rc = twIf_Transact(pFwDebug->hTwif,pTxn);

	return rc;
}


/*
 * \brief	Read Address to FW
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \param  Address - Absolute HW address
 * \param  Length - Length in byte to write
 * \param  Buffer - Buffer to copy to FW
 * \param  fCb - CB function
 * \param  hCb - CB Handle
 * \return none
 * 
 * \par Description
 * Read from HW, must receive length in byte max size 256 bytes
 * address must be absolute HW address.
 * 
 * \sa 
 */
TI_STATUS fwDbg_ReadAddr (TI_HANDLE hFwDebug,
                          TI_UINT32 Address,
                          TI_UINT32 Length,
                          TI_UINT8* Buffer,
                          TFwDubCallback fCb,
                          TI_HANDLE hCb)
{
	TI_STATUS rc;
	TTxnStruct *pTxn;
	TFwDebug *pFwDebug = (TFwDebug*)hFwDebug;
	pTxn = &pFwDebug->tTxn;
	/* check if length is large than default threshold */
	if (Length > DMA_SIZE_BUF)
	{
TRACE1(pFwDebug->hOs, REPORT_SEVERITY_ERROR, "fwDbg_ReadAddr : Buffer Length too large -- %d",Length);
		return TXN_STATUS_ERROR;
	}

	pFwDebug->fCb = fCb;
	pFwDebug->hCb = hCb;
	pFwDebug->pReadBuf = Buffer;

	/* Build the command TxnStruct */
    TXN_PARAM_SET(pTxn, TXN_LOW_PRIORITY, TXN_FUNC_ID_WLAN, TXN_DIRECTION_READ, TXN_INC_ADDR)
	/* Applying a CB in case of an async read */
    BUILD_TTxnStruct(pTxn, Address, pFwDebug->pDMABuf, Length,(TTxnDoneCb)fwDbg_ReadAddrCb, pFwDebug)
	rc = twIf_Transact(pFwDebug->hTwif,pTxn);
	if (rc == TXN_STATUS_COMPLETE)
    {
		/* copy from DMA buufer to given buffer */
		os_memoryCopy(pFwDebug->hOs,pFwDebug->pReadBuf,pFwDebug->pDMABuf,Length);
	}
	return rc;
}


/*
 * \brief	Write CB function
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \param  pTxn - pointer ti Transact
 * \return none
 * 
 * \par Description
 * This function called from TWIF upon Async Write
 * 
 * \sa 
 */
static void fwDbg_WriteAddrCb (TI_HANDLE hFwDebug,TTxnStruct* pTxn)
{
	TFwDebug *pFwDebug = (TFwDebug*)hFwDebug;

	if (pFwDebug->fCb && pFwDebug->hCb)
    {
		pFwDebug->fCb(pFwDebug->hCb);
    }
}


/*
 * \brief	Read CB function
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \param  pTxn - pointer ti Transact
 * \return none
 * 
 * \par Description
 * This function called from TWIF upon Async Read
 * 
 * \sa 
 */
static void fwDbg_ReadAddrCb (TI_HANDLE hFwDebug,TTxnStruct* pTxn)
{
	TFwDebug *pFwDebug = (TFwDebug*)hFwDebug;
	/* copy from DMA buufer to given buffer */
	os_memoryCopy(pFwDebug->hOs,pFwDebug->pReadBuf,pFwDebug->pDMABuf,pTxn->aLen[0]);

	if (pFwDebug->fCb && pFwDebug->hCb)
    {
		pFwDebug->fCb(pFwDebug->hCb);
    }
}
    

/*
 * \brief	Check HW address
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \return TI_TRUE, TI_FALSE
 * 
 * \par Description
 * This function called to check the given address to be a valid memory address.
 * 
 * \sa 
 */
TI_BOOL fwDbg_isValidMemoryAddr (TI_HANDLE hFwDebug, TI_UINT32 Address, TI_UINT32 Length)
{
	TFwDebug *pFwDebug = (TFwDebug*)hFwDebug; 

	return twIf_isValidMemoryAddr(pFwDebug->hTwif, Address, Length);
}


/*
 * \brief	Check HW address
 * 
 * \param  hFwDebug  - Handle to FW Debug
 * \return TI_TRUE, TI_FALSE
 * 
 * \par Description
 * This function called to check the given address to be a valid register address.
 * 
 * \sa 
 */
TI_BOOL fwDbg_isValidRegAddr (TI_HANDLE hFwDebug, TI_UINT32 Address, TI_UINT32 Length)
{
	TFwDebug *pFwDebug = (TFwDebug*)hFwDebug; 

	return twIf_isValidRegAddr(pFwDebug->hTwif, Address, Length);
}

