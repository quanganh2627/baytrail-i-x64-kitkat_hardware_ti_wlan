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

/***************************************************************************/
/*																			*/
/*	  MODULE:	ccxRMMngrParam.h											*/
/*    PURPOSE:	ccx v2 Radio Measurement module Parameters internal         */
/*              header file		                                            */
/*																			*/
/***************************************************************************/




#ifdef CCX_MODULE_INCLUDED

#ifndef __CCXRMMNGRPARAM_H__
#define __CCXRMMNGRPARAM_H__



#include "802_11Defs.h"
#include "TWDriver.h"



#define		RM_REQUEST_IE_ID	38
#define		MAX_RM_REPORT_SIZE		2000
#define     PARENT_TSF_LEN      4
#define     TARGET_TSF_LEN      8
#define		RADIO_MNGT_CAPABILITY_IE_ID     221
#define		RADIO_MNGT_MBSSID_MASK_MASK		0x0007



/* Not in use at the moment */
typedef enum
{
	NO_RM	= 0,
	RM_STATE_NORMAL	            = 1,
	RM_STATE_AP_RADIO_SCAN		= 2,
	RM_STATE_CLIENT_WALKABOUT	= 3
} rmState_e;



typedef struct 
{
	TI_UINT16				elementID;
	TI_UINT16				length;
	TI_UINT16				token;
	TI_UINT8				mode;
	TI_UINT8           	type;
} RM_report_IE_hdr_t;



typedef struct
{
    RM_report_IE_hdr_t      hdr;
	TI_UINT8				channelNum;
	TI_UINT8				spare;
	TI_UINT16				duratonTime;

	TI_UINT8			    phyType;
	TI_INT8			    	recvSignalPower;	/*rssi (casting from TI_INT32 to TI_INT8) */
	TMacAddr        		bssid;
	TI_UINT8				parentTSF[PARENT_TSF_LEN];
	TI_UINT8				targetTSF[TARGET_TSF_LEN];

	TI_UINT16 				beaconInerval;
	TI_UINT16 				capabilities;

    TI_UINT8                pInfoElements[sizeof(dot11_SSID_t)
                                        + sizeof(dot11_RATES_t)
                                        + sizeof(dot11_FH_PARAMS_t)
                                        + sizeof(dot11_DS_PARAMS_t)
                                        + sizeof(dot11_CF_PARAMS_t)
                                        + sizeof(dot11_TIM_t)
                                        + sizeof(dot11_IBSS_PARAMS_t)
                                        + sizeof(ccx_radioManagmentCapability_IE_t)];

} beacon_report_t;



typedef struct
{
    RM_report_IE_hdr_t  hdr;
	TI_UINT8				channelNum;
	TI_UINT8				spare;
	TI_UINT16				duratonTime;

	TI_UINT8				ccaBusyFraction;
} channelLoad_report_t;



typedef struct
{
    RM_report_IE_hdr_t  hdr;
	TI_UINT8				channelNum;
	TI_UINT8				spare;
	TI_UINT16				duratonTime;
	
	TI_UINT8				rpiDensity[MEASUREMENT_NOISE_HISTOGRAM_NUM_OF_RANGES];
} noiseHistogram_report_t;



typedef struct 
{
	TI_UINT16				dialogToken;
	TI_UINT8				reportElements[MAX_RM_REPORT_SIZE];
} RM_report_frame_t;



#endif /* __CCXRMMNGRPARAM_H__ */

#endif /* CCX_MODULE_INCLUDED */
