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
/** \file admCtrlCcx.h
 *  \brief Admission control for CCX API
 *
 *  \see admCtrl.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Admission Control                                             *
 *   PURPOSE: Admission Control Module API                                  *
 *                                                                          *
 ****************************************************************************/

#ifndef _ADM_CTRL_CCX_H_
#define _ADM_CTRL_CCX_H_

#ifdef CCX_MODULE_INCLUDED


/* Constants */

/* Enumerations */

/* Typedefs */


/* RSN admission control prototypes */


/* Structures */


#define RSN_AIRONET_FLAGS_OFFSET    10
#define AIRONET_MIC_ENABLED         0x08
#define AIRONET_KP_ENABLED          0x10

#define CKIP_ONLY   0x00
#define CKIP_CMIC   0x01
#define CMIC_ONLY   0x02 

#define DOT11_AIRO_PROXY_ARP        0X40
/*typedef struct
{

    TI_UINT8 elementid;         < Aironet IE element ID - should always be 0x85 
    TI_UINT8 length;                < Aironet IE length - at least 28 bytes 
    TI_UINT8 oui[4];
    TI_UINT16 cwMin;
    TI_UINT16 cwMax;
    TI_UINT8 Flags;            CCX Flags: KP and MIC
    TI_UINT8 Reserved[21];
} aironetIePacket_t;*/

#define DOT11_SYS_NAME_LEN 15

typedef struct
{

    TI_UINT8    elementid;          /*< Aironet IE element ID - should always be 0x85 */
    TI_UINT8    length;             /*< Aironet IE length - at least 28 bytes */ 
    TI_UINT8    load;
    TI_UINT8    hops;
    TI_UINT8    device;
    TI_UINT8    refresh_rate;
    TI_UINT16   cwMin;
    TI_UINT16   cwMax;
    TI_UINT8    Flags;            /*CCX Flags: KP and MIC*/
    TI_UINT8    distance;
    TI_UINT8    name[DOT11_SYS_NAME_LEN+1];
    TI_UINT16   num_of_assoc;
    TI_UINT16   radiotype;
    
    /*TI_UINT8  Reserved[21];*/
} aironetIePacket_t;



#define RSN_AP_IP_ADDRESS_OFFSET 6

typedef struct
{
    TI_UINT8 elementid;         /*< Aironet IE element ID - should always be 0x85 */
    TI_UINT8 length;                /*< Aironet IE length - at least 28 bytes */
    TI_UINT8 oui[4];
    TI_UINT8 IpAdress[4];
    TI_UINT8 Reserved[2];
} apIpAddrIePacket_t;



/* CCX capabilities structure */
typedef struct 
{
    TI_UINT8    micSupport;
    TI_UINT8    kpSupport;
    TI_UINT8    reserved[8];
    TI_UINT8    proxyArpEnabled;
    TI_UINT8    refreshRate;
} aironetIeData_t;

/* External data definitions */

/* External functions definitions */

/* Function prototypes */


TI_STATUS admCtrlCcx_evalSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TRsnSiteParams *pRsnSiteParams, TI_UINT32 *pEvaluation, TI_BOOL *pCcxSupport);
TI_STATUS admCtrlCcx_setSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TI_UINT8 *pAssocIe, TI_UINT8 *pAssocIeLen);
TI_STATUS admCtrlCcx_getInfoElement(admCtrl_t *pAdmCtrl, TI_UINT8 *pIe, TI_UINT8 *pLength);
TI_UINT32 admCtrlCcx_parseCckmSuiteVal(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal);
TI_UINT32  admCtrlCcx_parseCckmSuiteVal4Wpa2(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal);
TI_BOOL admCtrlCcx_getCckmAkm(admCtrl_t *pAdmCtrl, TI_UINT8 *akmSuite);
TI_STATUS admCtrlCcx_setExtendedParams(admCtrl_t *pAdmCtrl, TRsnData *pRsnData);
TI_STATUS admCtrlCcx_getWpaCipherInfo(admCtrl_t *pAdmCtrl, wpaIePacket_t *pWpaIePacket);
TI_UINT32  admCtrlCcx_WpaParseSuiteVal(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal, wpaIeData_t *pWpaData);


#endif /*CCX_MODULE_INCLUDED*/

#endif /*  _ADM_CTRL_CCX_H_*/

