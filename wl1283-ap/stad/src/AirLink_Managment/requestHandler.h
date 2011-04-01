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
/** \file requestHandler.h
 *  \brief Request Handler module interface header file
 *
 *  \see requestHandler.c
 */

/***************************************************************************/
/*                                                                          */
/*    MODULE:   requestHandler.h                                            */
/*    PURPOSE:  Request Handler module interface header file                */
/*                                                                          */
/***************************************************************************/
#ifndef __REQUEST_HANDLER_H__
#define __REQUEST_HANDLER_H__

#include "paramOut.h"
#include "802_11Defs.h"
#include "measurementMgrApi.h"


typedef struct 
{
    EMeasurementType            Type;
    TI_BOOL                     isParallel; 
    TI_UINT16                   frameToken; 
    TI_UINT16                   measurementToken;
    TI_UINT8                    channelNumber;
    TI_UINT16                   DurationTime;
    TI_UINT8                    ActualDurationTime;
    EMeasurementScanMode        ScanMode;
} MeasurementRequest_t;


/* Functions Pointers Definitions */
typedef TI_STATUS (*parserRequestIEHdr_t)   (TI_UINT8 *pData, TI_UINT16 *reqestLen,
                                             TI_UINT16 *measurementToken);

typedef struct 
{
    /* Function to the Pointer */
    parserRequestIEHdr_t    parserRequestIEHdr;

    /* General Params */
    MeasurementRequest_t    reqArr[MAX_NUM_REQ];
    TI_UINT8                numOfWaitingRequests;   
    TI_INT8                 activeRequestID;

    /* Handlers */
    TI_HANDLE               hReport;
    TI_HANDLE               hOs;
} requestHandler_t;



TI_HANDLE requestHandler_create(TI_HANDLE hOs);

TI_STATUS RequestHandler_config(TI_HANDLE   hRequestHandler,
                                TI_HANDLE       hReport,
                                TI_HANDLE       hOs);

TI_STATUS requestHandler_setParam(TI_HANDLE hRequestHandler,
                                  paramInfo_t   *pParam);

TI_STATUS requestHandler_getParam(TI_HANDLE     hRequestHandler,
                                            paramInfo_t *pParam);

TI_STATUS requestHandler_destroy(TI_HANDLE hRequestHandler);

TI_STATUS requestHandler_insertRequests(TI_HANDLE hRequestHandler,
                                        EMeasurementMode measurementMode,
                                        TMeasurementFrameRequest measurementFrameReq);

TI_STATUS requestHandler_getNextReq(TI_HANDLE hRequestHandler,
                                    TI_BOOL   isForActivation,
                                    MeasurementRequest_t *pRequest[],
                                    TI_UINT8*     numOfRequests);

TI_STATUS requestHandler_getCurrentExpiredReq(TI_HANDLE hRequestHandler,
                                              TI_UINT8 requestIndex,
                                              MeasurementRequest_t **pRequest);

TI_STATUS requestHandler_clearRequests(TI_HANDLE hRequestHandler);

TI_STATUS requestHandler_getFrameToken(TI_HANDLE hRequestHandler,TI_UINT16 *frameToken );

TI_STATUS requestHandler_setRequestParserFunction(TI_HANDLE hRequestHandler, 
                                                  parserRequestIEHdr_t parserRequestIEHdr);


#endif /* __REQUEST_HANDLER_H__*/
