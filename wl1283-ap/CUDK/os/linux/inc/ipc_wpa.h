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

/****************************************************************************/
/*                                                                          */
/*    MODULE:   Ipc_Wpa.h                                                   */
/*    PURPOSE:                                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _IPC_WPA_H_
#define _IPC_WPA_H_

/* defines */
/***********/
#define IPC_WPA_RESP_MAX_LEN 256

/* types */
/*********/

/* functions */
/*************/
THandle IpcWpa_Create(PS32 pRes, PS8 pSupplIfFile);
VOID IpcWpa_Destroy(THandle hIpcWpa);

S32 IpcWpa_Command(THandle hIpcWpa, PS8 cmd, S32 print);
S32 IpcWpa_CommandWithResp(THandle hIpcWpa, PS8 cmd, S32 print, PS8 pResp, PU32 pRespLen);


#endif  /* _IPC_WPA_H_ */
        
