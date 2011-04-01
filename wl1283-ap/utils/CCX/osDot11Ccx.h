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
/* Module:		osDot11Ccx.h */
/**/
/* Purpose:		*/
/**/
/*--------------------------------------------------------------------------*/
#ifndef __OSDOT11CCX_H__
#define __OSDOT11CCX_H__


#ifdef CCX_MODULE_INCLUDED

/************************************************************************
 * CCX types  - taken from Funk inc file: fswccx.h                      *
 ************************************************************************/

/* Rogue AP structure */

typedef struct _OS_CCX_ROGUE_AP_DETECTED
{
	TI_UINT16             FailureReason;
	TMacAddr              RogueAPMacAddress;
	char                  RogueAPName[16];
} OS_CCX_ROGUE_AP_DETECTED, *POS_CCX_ROGUE_AP_DETECTED;

/**/
/*  Auth Success structure*/
/**/


typedef struct
{
	TI_UINT32 SsidLength;
    TI_UINT8  Ssid[32];
} OS_CCX_802_11_SSID;


typedef struct _OS_CCX_AUTH_SUCCESS
{
  	OS_CCX_802_11_SSID      Ssid;
	TMacAddr                BSSID;
} OS_CCX_AUTH_SUCCESS, *POS_CCX_AUTH_SUCCESS;


/**/
/*  CCKM Request structure*/
/**/

/* RequestCode values*/
typedef enum _OS_CCX_CCKM_REQUEST_CODE
{
	Ccx_CckmFirstTime = 0,
	Ccx_CckmFastHandoff
} OS_CCX_CCKM_REQUEST_CODE;

typedef struct _OS_CCX_CCKM_REQUEST
{
	OS_CCX_CCKM_REQUEST_CODE    RequestCode;
	TI_UINT32                    AssociationRequestIELength;
	TI_UINT8                     AssociationRequestIE[1];
} OS_CCX_CCKM_REQUEST;

typedef struct _ccx_radioManagmentCapability_IE_t
{
	TI_UINT8			eleID;
	TI_UINT8			len;
	TI_UINT8			ciscoAironetOUI[3];
	TI_UINT8			version;
	TI_UINT16  		rmState;
} ccx_radioManagmentCapability_IE_t;

#define     OS_CCX_CONFIGURATION_ENABLE_CKIP        0x0001
#define     OS_CCX_CONFIGURATION_ENABLE_ROGUE_AP    0x0002
#define     OS_CCX_CONFIGURATION_ENABLE_CCKM        0x0004

#define     OS_CCX_CONFIGURATION_ENABLE_ALL         0x0007 



typedef enum _OS_CCX_NETWORK_EAP
{
    OS_CCX_NETWORK_EAP_OFF  = 0,
    OS_CCX_NETWORK_EAP_ON,
    OS_CCX_NETWORK_EAP_ALLOWED,
    OS_CCX_NETWORK_EAP_PREFERRED
} OS_CCX_NETWORK_EAP;

/**/
/*  CCKM Result structure*/
/**/

/* ResultCode values*/
typedef enum _OS_CCX_CCKM_RESULT_CODE
{
	osCcx_CckmSuccess = 0,
	osCcx_CckmFailure,
	osCcx_CckmNotInUse
} OS_CCX_CCKM_RESULT_CODE;

typedef struct _OS_CCX_CCKM_RESULT
{
	OS_CCX_CCKM_RESULT_CODE ResultCode;
} OS_CCX_CCKM_RESULT;


/**/
/*  CCKM Start structure*/
/**/

typedef struct _OS_CCX_CCKM_START
{
	TI_UINT8 Timestamp[8];
	char BSSID[6];
} OS_CCX_CCKM_START;

/*
	GUIDs for custom OIDs in #define form
	-------------------------------------

	The same GUIDs constructed using DEFINE_GUID (above) are also 
	available in #define form. This allows a GUID structure to be 
	constructed directly. For example, an NDIS_GUID structure could be 
	constructed as follows:

		NDIS_GUID ng = {CGUID_FSW_CCX_CONFIGURATION, OID_FSW_CCX_CONFIGURATION, 4, fNDIS_GUID_TO_OID};
 */

/* oids*/
#define CGUID_FSW_CCX_CONFIGURATION				{0x21190696, 0x118d, 0x4654, {0x9e, 0x9a, 0xc6, 0x9c, 0xa7, 0xc7, 0x95, 0xb8}}
#define CGUID_FSW_CCX_NETWORK_EAP				{0x0725e492, 0x3025, 0x477c, {0x91, 0xdc, 0xd5, 0xc1, 0x2a, 0x4e, 0xec, 0x1f}}
#define CGUID_FSW_CCX_ROGUE_AP_DETECTED			{0x5858fa82, 0x0dfd, 0x4a4a, {0xbb, 0xc9, 0xdc, 0xc7, 0x8f, 0x63, 0x01, 0x70}}
#define CGUID_FSW_CCX_REPORT_ROGUE_APS			{0x6e72993a, 0x59a7, 0x4a3e, {0xb1, 0x65, 0x0c, 0xec, 0xb3, 0xc5, 0x0c, 0xdc}}
#define CGUID_FSW_CCX_AUTH_SUCCESS				{0x55019653, 0x0454, 0x4309, {0xb8, 0xca, 0xd2, 0xe9, 0xf4, 0xd0, 0xaf, 0x83}}
#define CGUID_FSW_CCX_CCKM_REQUEST				{0xf5190942, 0x6d90, 0x4858, {0x8a, 0xdf, 0x08, 0x6a, 0x2f, 0xa5, 0xb7, 0xeb}}
#define CGUID_FSW_CCX_CCKM_RESULT				{0x1163fca7, 0x9c1a, 0x4e39, {0xa8, 0x79, 0x9f, 0x93, 0xad, 0x1b, 0x84, 0x07}}

/* status code*/
#define CGUID_FSW_CCX_CCKM_START				{0x8c389e47, 0xe511, 0x4d96, {0xae, 0xfe, 0x2f, 0xb7, 0x31, 0xd8, 0x0c, 0x05}}

#define OID_FSW_CCX_CONFIGURATION				0xFF010201
#define OID_FSW_CCX_NETWORK_EAP					0xFF010202
#define OID_FSW_CCX_ROGUE_AP_DETECTED			0xFF010203
#define OID_FSW_CCX_REPORT_ROGUE_APS			0xFF010204
#define OID_FSW_CCX_AUTH_SUCCESS				0xFF010205
#define OID_FSW_CCX_CCKM_REQUEST				0xFF010206
#define OID_FSW_CCX_CCKM_RESULT					0xFF010207
/* status code*/
#define NDIS_STATUS_FSW_CCX_CCKM_START			0x60010001



#endif /*CCX_MODULE_INCLUDED*/

#endif /*__OSDOT11CCX_H__*/
