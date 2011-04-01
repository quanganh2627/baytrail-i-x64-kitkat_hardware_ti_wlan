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
 * inc/windows_types.h
 *
 */

#ifndef _WINDOWS_TYPES_H
#define _WINDOWS_TYPES_H

#include "tidef.h"
#include "ioctl_init.h"

/*typedef TI_BOOL    TI_BOOL, *PBOOLEAN;*/
typedef TI_UINT16    USHORT, *PUSHORT;
typedef char      CHAR;
typedef const char * LPCSTR;

#define IN
#define OUT
#define NDIS_MAX_STRING_LEN   361

typedef struct _STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  TI_INT8*  Buffer;
} ANSI_STRING, *PANSI_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef ANSI_STRING NDIS_STRING, *PNDIS_STRING;
typedef void * NDIS_HANDLE;
typedef int NDIS_STATUS, *PNDIS_STATUS;


typedef TI_UINT32 NDIS_OID, *PNDIS_OID;
#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
/*typedef LARGE_INTEGER NDIS_PHYSICAL_ADDRESS; */
typedef void* *PDEVICE_OBJECT, *PDRIVER_OBJECT;
typedef TI_UINT32 NDIS_MINIPORT_INTERRUPT, NDIS_MINIPORT_TIMER, NDIS_SPIN_LOCK;
typedef TI_UINT32 PNDIS_PACKET, *PPNDIS_PACKET;

typedef enum _NDIS_PARAMETER_TYPE {
  NdisParameterInteger,
  NdisParameterHexInteger,
  NdisParameterString,
  NdisParameterMultiString,
  NdisParameterBinary
} NDIS_PARAMETER_TYPE, *PNDIS_PARAMETER_TYPE;

typedef struct {
    USHORT  Length;
    void*  Buffer;
} BINARY_DATA;

typedef struct _NDIS_CONFIGURATION_PARAMETER {
  NDIS_PARAMETER_TYPE  ParameterType;
  union {
    TI_UINT32  IntegerData;
    NDIS_STRING  StringData;
    BINARY_DATA  BinaryData;
  } ParameterData;
  char StringBuffer[NDIS_MAX_STRING_LEN];
} NDIS_CONFIGURATION_PARAMETER, *PNDIS_CONFIGURATION_PARAMETER;

typedef TI_UINT32 NTSTATUS;

#ifndef NDIS_STATUS_SUCCESS
# define NDIS_STATUS_SUCCESS                        ((NDIS_STATUS)0x00000000L)
# define NDIS_STATUS_PENDING                        ((NDIS_STATUS)0x00000103L)
# define NDIS_STATUS_RESET_END                      ((NDIS_STATUS)0x40010005L)
# define NDIS_STATUS_MEDIA_SPECIFIC_INDICATION      ((NDIS_STATUS)0x40010012L)
# define NDIS_STATUS_FAILURE                        ((NDIS_STATUS)0xC0000001L)
# define NDIS_STATUS_ADAPTER_NOT_FOUND              ((NDIS_STATUS)0xC0010006L)
# define NDIS_STATUS_INVALID_LENGTH                 ((NDIS_STATUS)0xC0010014L)
# define NDIS_STATUS_BUFFER_TOO_SHORT               ((NDIS_STATUS)0xC0010016L)
# define NDIS_STATUS_INVALID_OID                    ((NDIS_STATUS)0xC0010017L)
#endif        /* NDIS_STATUS_SUCCESS */

#define STATUS_SUCCESS                0
#define STATUS_INVALID_PARAMETER     -1

#define NdisZeroMemory(p, size)      os_memoryZero( NULL, p, size )
#define NdisMoveMemory(d, s, size)   os_memoryCopy( NULL, d, s, size )

NDIS_STATUS NdisUnicodeStringToAnsiString( IN OUT PANSI_STRING  DestinationString,
    IN PUNICODE_STRING  SourceString );
void NdisReadConfiguration( OUT PNDIS_STATUS  Status, OUT PNDIS_CONFIGURATION_PARAMETER  *ParameterValue,
    IN NDIS_HANDLE  ConfigurationHandle, IN PNDIS_STRING  Keyword, IN NDIS_PARAMETER_TYPE  ParameterType );
void NdisWriteConfiguration( OUT PNDIS_STATUS  Status, IN NDIS_HANDLE  ConfigurationHandle,
    IN PNDIS_STRING  Keyword, IN PNDIS_CONFIGURATION_PARAMETER  ParameterValue );

typedef struct _NDIS_PACKET_POOL {
  NDIS_SPIN_LOCK  SpinLock;
  struct _NDIS_PACKET *FreeList;
  TI_UINT32  PacketLength;
  TI_UINT8  Buffer[1];
} NDIS_PACKET_POOL, * PNDIS_PACKET_POOL;

typedef enum _NDIS_802_11_STATUS_TYPE
{
    Ndis802_11StatusType_Authentication,
    Ndis802_11StatusTypeMax    /* not a real type, defined as an upper bound*/
} NDIS_802_11_STATUS_TYPE, *PNDIS_802_11_STATUS_TYPE;

typedef TI_UINT8   NDIS_802_11_MAC_ADDRESS[6];

typedef struct _NDIS_802_11_STATUS_INDICATION
{
    NDIS_802_11_STATUS_TYPE StatusType;
} NDIS_802_11_STATUS_INDICATION, *PNDIS_802_11_STATUS_INDICATION;

typedef struct _NDIS_802_11_AUTHENTICATION_REQUEST
{
    TI_UINT32 Length;            /* Length of structure*/
    NDIS_802_11_MAC_ADDRESS Bssid;
    TI_UINT32 Flags;
} NDIS_802_11_AUTHENTICATION_REQUEST, *PNDIS_802_11_AUTHENTICATION_REQUEST;

typedef TI_INT32 NDIS_802_11_RSSI;           /* in dBm*/

typedef struct _NDIS_802_11_TEST
{
	TI_UINT32 Length;
	TI_UINT32 Type;
	union {
		struct _AuthenticationEvent {
            NDIS_802_11_STATUS_INDICATION Status;
            NDIS_802_11_AUTHENTICATION_REQUEST Request[1];
		} AuthenticationEvent;
		NDIS_802_11_RSSI RssiTrigger;
	};
} NDIS_802_11_TEST, *PNDIS_802_11_TEST;

/* Added new encryption types*/
/* Also aliased typedef to new name*/
typedef enum _NDIS_802_11_WEP_STATUS
{
    Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS,
  NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

#ifdef TI_DBG
#ifdef __KERNEL__
#define DbgPrint     printk
#else
#define DbgPrint     printf
#endif
#else
#define DbgPrint
#endif

#endif /* _WINDOWS_TYPES_H */
