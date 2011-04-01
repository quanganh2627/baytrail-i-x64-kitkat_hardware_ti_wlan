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

/*
 * src/osRgstry_parser.h 
 *
 */

#ifndef _OS_RGSTRY_PARSER_
#define _OS_RGSTRY_PARSER_

#include "ioctl_init.h"

#include "tidef.h"
#include "osApi.h"
#include "osDebug.h"



#define NDIS_STRING_CONST(a)    {sizeof(a), sizeof(a), a}
#include "WlanDrvIf.h"
#include "WlanDrvIf.h"

void regFillInitTable ( TWlanDrvIfObjPtr pAdapter, void* pInitTable);

extern void osInitTable (TInitTable *InitTable);
extern  int osInitTable_IniFile (TI_HANDLE hOs, TInitTable *InitTable, char *file_buf, int file_length);


#endif /* _OS_RGSTRY_PARSER_ */

