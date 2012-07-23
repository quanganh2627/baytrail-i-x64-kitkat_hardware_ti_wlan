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
#ifndef _OS_TRANS_H
#define _OS_TRANS_H

#include "cu_osapi.h"
#include "tidef.h"

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#define MAX_QUEUE_LENGTH	5//maximum length the queue of pending connections

typedef  int		SOCKET;

TI_BOOL os_trans_create(VOID);
TI_BOOL os_socket (THandle* pSock);
TI_BOOL os_bind (THandle sock, U16 port);
TI_BOOL os_sockWaitForConnection (THandle socket_id, THandle* pConnSock);
TI_BOOL os_sockSend (THandle socket_id, PS8 buffer, U32 bufferSize);
VOID	os_trans_destroy(VOID);
S32		os_sockRecv (THandle socket_id, PU8 pBuffer, U32 bufferSize, TI_SIZE_T flags);

#endif
