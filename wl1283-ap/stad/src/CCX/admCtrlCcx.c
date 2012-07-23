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
/** \file admCtrlWep.c
 *  \brief Admission control API implimentation
 *
 *  \see admCtrl.h
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Admission Control                                             *
 *   PURPOSE: Admission Control Module API                                  *
 *                                                                          *
 ****************************************************************************/
#ifdef CCX_MODULE_INCLUDED

#define __FILE_ID__  FILE_ID_10
#include "osApi.h"
#include "paramOut.h"
#include "fsm.h"
#include "report.h"
#include "mlmeApi.h"
#include "DataCtrl_Api.h"
#include "rsnApi.h"
#include "rsn.h"
#include "admCtrl.h"
#include "admCtrlWpa.h"
#include "admCtrlCcx.h"
#include "ccxMngr.h"
#include "PowerMgr_API.h"
#include "admCtrlWpa2.h"
#include "TWDriver.h"
#include "currBss.h"

/* Constants */

#define CCX_IE_KEY_MNG_CCKM     0
#define CCX_RADIO_TYPE_4500     0x66

/* Enumerations */

/* Typedefs */

/* Structures */

/* External data definitions */

/* Local functions definitions */

/* Global variables */

/* Function prototypes */
TI_STATUS admCtrlCcx_parseAironetIe(admCtrl_t *pAdmCtrl, TI_UINT8 *pAironetIe, aironetIeData_t *pAironetData);
TI_STATUS admCtrlCcx_parseApIpAddrIe(admCtrl_t *pAdmCtrl, TI_UINT8 *pApIptIe, TI_UINT8 *pApIpData);


/**
*
* admCtrlCcx_getInfoElement - Build the current information element.
*
* \b Description: 
*
* Build the current information element.
*
* \b ARGS:
*
*  I   - pAdmCtrl - context \n
*  I   - pIe - IE buffer \n
*  I   - pLength - length of IE \n
*  
* \b RETURNS:
*
*  TI_OK on success, TI_NOK on failure.
*                             
* \sa 
*/
TI_STATUS admCtrlCcx_getInfoElement(admCtrl_t *pAdmCtrl, TI_UINT8 *pIe, TI_UINT8 *pLength)
{
    aironetIePacket_t   *pAironetleData;
    apIpAddrIePacket_t  *pApIpAddrIeData;
    static TI_UINT8 apIpOuiIe[4] = { 0x00, 0x40, 0x96, 0x00};

    if ((pIe==NULL)|| (!pAdmCtrl->ccxSupport))  
    {
        *pLength = 0;
        return TI_OK;
    }
    /* Build Aironet IE */
    pAironetleData = (aironetIePacket_t*)pIe;
    os_memoryZero(pAdmCtrl->hOs, pAironetleData, sizeof(aironetIePacket_t));
    pAironetleData->elementid= CCX_EXT_1_IE_ID;
    pAironetleData->length = sizeof(aironetIePacket_t)-2;

    /* update to refresh_rate */
    currBSS_GetDefaultKeepAlivePeriod (pAdmCtrl->hCurrBss, &pAironetleData->refresh_rate);

    /* set the device number to be CCX_RADIO_TYPE_4500 */
    pAironetleData->device = CCX_RADIO_TYPE_4500;
    
    /* Add the capabilities */
    pAironetleData->Flags |= 
#ifdef CKIP_ENABLED /* In case ckip is not enabled we do not send the AiroNet InfoElement.*/     
        AIRONET_MIC_ENABLED | AIRONET_KP_ENABLED | 
#endif      
        DOT11_AIRO_PROXY_ARP ;

#ifndef DO_NOT_SEND_CCX_EXT_2_IE
    /* Add the AP IP Address */
    pApIpAddrIeData = (apIpAddrIePacket_t*) ((TI_UINT8*)pIe+pAironetleData->length+2);
    os_memoryZero(pAdmCtrl->hOs, pApIpAddrIeData, sizeof(apIpAddrIePacket_t));
    pApIpAddrIeData->elementid = CCX_EXT_2_IE_ID;
    pApIpAddrIeData->length = sizeof(apIpAddrIePacket_t)-2;
    os_memoryCopy(pAdmCtrl->hOs, pApIpAddrIeData->oui, apIpOuiIe, 4);
    *pLength = pAironetleData->length + pApIpAddrIeData->length+4;
#else
    *pLength = pAironetleData->length + 2;
#endif

    return TI_OK;
}

/**
*
* admCtrlCcx_setSite  - Set current primary site parameters for registration.
*
* \b Description: 
*
* Set current primary site parameters for registration.
*
* \b ARGS:
*
*  I   - pAdmCtrl - context \n
*  I   - pRsnData - site's RSN data \n
*  O   - pAssocIe - result IE of evaluation \n
*  O   - pAssocIeLen - length of result IE of evaluation \n
*  
* \b RETURNS:
*
*  TI_OK on site is aproved, TI_NOK on site is rejected.
*
* \sa 
*/
TI_STATUS admCtrlCcx_setSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TI_UINT8 *pAssocIe, TI_UINT8 *pAssocIeLen)
{
    TI_STATUS           status;
    paramInfo_t         param;
    TTwdParamInfo       tTwdParam;
    aironetIeData_t     aironetIeData;
    TI_UINT8            ApIpAdress[4];
    TI_UINT8            *pAironetIe=NULL;
    TI_UINT8            *pApIpAddrData=NULL;

    if (pRsnData==NULL)
    {
        return TI_NOK;
    }
    os_memoryZero(pAdmCtrl->hOs, &aironetIeData, sizeof(aironetIeData));

    /* If we get here at first time (i.e. during scanning process) then  */
    /* we need tp perform  some privacy settings                         */
    if (!pAdmCtrl->setSiteFirst)
    {

        /* Configure privacy status in HAL so that HW is prepared to recieve keys */
        tTwdParam.paramType = TWD_RSN_SECURITY_MODE_PARAM_ID;   
        tTwdParam.content.rsnEncryptionStatus = (ECipherSuite)TWD_CIPHER_WEP;
        status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);
        if (status != TI_OK)
        {
            return status;
        }

        /* clean MIC and KP flags in HAL and re-send WEP-keys        */
        tTwdParam.paramType = TWD_RSN_CCX_SW_ENC_ENABLE_PARAM_ID; 
        tTwdParam.content.rsnCcxSwEncFlag = TI_FALSE;
        status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);

        tTwdParam.paramType = TWD_RSN_CCX_MIC_FIELD_ENABLE_PARAM_ID; 
        tTwdParam.content.rsnCcxMicFieldFlag = TI_FALSE;
        status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);
        if (pAdmCtrl->authSuite != RSN_AUTH_OPEN)
        {
            rsn_setDefaultKeys(pAdmCtrl->pRsn);
        }
    }

    status = admCtrl_parseIe(pAdmCtrl, pRsnData, &pAironetIe, CCX_EXT_1_IE_ID);
    if (status != TI_OK)                                                         
    {                                                                                    
        return status;                                                        
    }
    if (pAironetIe!=NULL)
    {   
        pAdmCtrl->ccxSupport = TI_TRUE;
        /* check CKIP */ 
        status = admCtrl_parseIe(pAdmCtrl, pRsnData, &pApIpAddrData, CCX_EXT_2_IE_ID);
        if (status != TI_OK)
        {
            return status;
        }
        
        status = admCtrlCcx_parseAironetIe(pAdmCtrl, pAironetIe, &aironetIeData);
        if (status != TI_OK)
        {
            return status;
        }
        
        status = admCtrlCcx_parseApIpAddrIe(pAdmCtrl, pApIpAddrData, ApIpAdress);
        if (status==TI_OK)
        {
           os_memoryCopy(pAdmCtrl->hOs ,pAdmCtrl->AP_IP_Address, ApIpAdress, 4);    
        }
        else
        {
            return status;
        }
    }
    else
    {   /* WEP */
        pAdmCtrl->ccxSupport = TI_FALSE;
        status = admCtrl_subConfig(pAdmCtrl);
    }
#ifdef CKIP_ENABLED
    /* set cipher suites */
    if ((!aironetIeData.kpSupport && !aironetIeData.micSupport) || !pAdmCtrl->setSiteFirst)
    {
        pAdmCtrl->unicastSuite = CIPHER_WEP;
        pAdmCtrl->broadcastSuite = CIPHER_WEP;
    }
    else
    {
        pAdmCtrl->unicastSuite  =  CIPHER_CKIP;
        pAdmCtrl->broadcastSuite =  CIPHER_CKIP;
    }
    pAdmCtrl->encrInSw = (aironetIeData.kpSupport) ? TI_TRUE : TI_FALSE;

#else
    pAdmCtrl->unicastSuite = TWD_CIPHER_WEP;
    pAdmCtrl->broadcastSuite = TWD_CIPHER_WEP;
    pAdmCtrl->encrInSw = TI_FALSE;
    aironetIeData.kpSupport = 0;
    aironetIeData.micSupport = 0;
#endif

    os_memoryCopy(pAdmCtrl->hOs, pAdmCtrl->aironetIeReserved, aironetIeData.reserved, 8);
    
    pAdmCtrl->proxyArpEnabled = (aironetIeData.proxyArpEnabled==1) ? TI_TRUE: TI_FALSE;

    *pAssocIeLen = pRsnData->ieLen;
    if (pAssocIe != NULL)
    {
        os_memoryCopy(pAdmCtrl->hOs, pAssocIe, &aironetIeData, sizeof(aironetIeData_t));
    } 

    /* Now we configure the MLME module with the 802.11 legacy authentication suite, 
        THe MLME will configure later the authentication module */
    param.paramType = MLME_LEGACY_TYPE_PARAM;
    if (pAdmCtrl->networkEapMode!=OS_CCX_NETWORK_EAP_OFF)
    {
        param.content.mlmeLegacyAuthType = AUTH_LEGACY_RESERVED1;
    }
    else
    {
        switch (pAdmCtrl->authSuite)
        {
        case RSN_AUTH_OPEN:
            param.content.mlmeLegacyAuthType = AUTH_LEGACY_OPEN_SYSTEM;
            break;

        case RSN_AUTH_SHARED_KEY: 
            param.content.mlmeLegacyAuthType = AUTH_LEGACY_SHARED_KEY;
            break;

        case RSN_AUTH_AUTO_SWITCH:
            param.content.mlmeLegacyAuthType = AUTH_LEGACY_AUTO_SWITCH;
            TRACE0(pAdmCtrl->hReport , REPORT_SEVERITY_INFORMATION, "CCX admCtrl mlme_setParam, RSN_AUTH_AUTO_SWITCH\n");
            break;

        default:
            return TI_NOK;
        }
    }
    status = mlme_setParam(pAdmCtrl->hMlme, &param);
    if (status != TI_OK)
    {
        return status;
    }

    param.paramType = RX_DATA_EAPOL_DESTINATION_PARAM;
    param.content.rxDataEapolDestination = OS_ABS_LAYER;
    status = rxData_setParam(pAdmCtrl->hRx, &param);
    if (status != TI_OK)
    {
        return status;
    }

    if (!pAdmCtrl->setSiteFirst)
    {
        pAdmCtrl->setSiteFirst = TI_TRUE;
        TRACE0(pAdmCtrl->hReport , REPORT_SEVERITY_INFORMATION, "CCX admCtrl setSite, first time\n");
        return TI_OK;
    }

    pAdmCtrl->setSiteFirst = TI_FALSE;

    TRACE0(pAdmCtrl->hReport , REPORT_SEVERITY_INFORMATION, "CCX admCtrl setSite, second time\n");

#ifdef CCX_MODULE_INCLUDED
    /* Configure privacy status in HAL so that HW is prepared to recieve keys */
    tTwdParam.paramType = TWD_RSN_SECURITY_MODE_PARAM_ID;   
    tTwdParam.content.rsnEncryptionStatus = (ECipherSuite)pAdmCtrl->unicastSuite;
    status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);
    if (status != TI_OK)
    {
        return status;
    }
    
    /* set MIC and KP in HAL  */
    tTwdParam.paramType = TWD_RSN_CCX_SW_ENC_ENABLE_PARAM_ID; 
    tTwdParam.content.rsnCcxSwEncFlag = aironetIeData.kpSupport;
    status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);
    if (status != TI_OK)
    {
        return status;
    }
    tTwdParam.paramType = TWD_RSN_CCX_MIC_FIELD_ENABLE_PARAM_ID; 
    tTwdParam.content.rsnCcxMicFieldFlag = aironetIeData.micSupport;
    status = TWD_SetParam(pAdmCtrl->pRsn->hTWD, &tTwdParam);
    
    if (status != TI_OK)
    {
        return status;
    }

#endif /*CCX_MODULE_INCLUDED*/
    /* re-config PAE */
    
    TRACE1(pAdmCtrl->hReport , REPORT_SEVERITY_INFORMATION, "CCX admCtrl setSite, cipher suite=%d\n", pAdmCtrl->unicastSuite);
    status = admCtrl_subConfig(pAdmCtrl);
    return status;
}

/**
*
* admCtrlCcx_evalSite  - Evaluate site for registration.
*
* \b Description: 
*
* evaluate site RSN capabilities against the station's cap.
* If the BSS type is infrastructure, the station matches the site only if it's WEP status is same as the site
* In IBSS, it does not matter
*
* \b ARGS:
*
*  I   - pAdmCtrl - Context \n
*  I   - pRsnData - site's RSN data \n
*  O   - pEvaluation - Result of evaluation \n
*  
* \b RETURNS:
*
*  TI_OK 
*
* \sa 
*/
TI_STATUS admCtrlCcx_evalSite(admCtrl_t *pAdmCtrl, TRsnData *pRsnData, TRsnSiteParams *pRsnSiteParams, TI_UINT32 *pEvaluation, TI_BOOL *pCcxSupport)
{
    
    aironetIeData_t     aironetIeData;
    TI_STATUS           status;
    TI_UINT8            *pAironetIe=NULL;

    *pEvaluation = 0;
    
    if (pRsnData==NULL)
    {
        return TI_NOK;
    }
    pAdmCtrl->setSiteFirst = TI_FALSE;
    
    os_memoryZero(pAdmCtrl->hOs, &aironetIeData, sizeof(aironetIeData));

    TRACE0(pAdmCtrl->hReport , REPORT_SEVERITY_INFORMATION, "CCX admCtrl evalSite \n");

    if (pRsnData->pIe==NULL) 
    {
        return TI_OK;
    }

    status = admCtrl_parseIe(pAdmCtrl, pRsnData, &pAironetIe, CCX_EXT_1_IE_ID);
    if (status != TI_OK)                                                         
    {                                                                                    
        return status;                                                        
    }
    if (pAironetIe==NULL)
    {   /* Regular WEP */
        *pCcxSupport = 0;
        *pEvaluation = 1;
    }
    else
    {   /* CKIP */
        status = admCtrlCcx_parseAironetIe(pAdmCtrl, pAironetIe, &aironetIeData);
        if (status!=TI_OK)
        {
            return status;
        }
        *pCcxSupport = 1;

        if (!aironetIeData.kpSupport && !aironetIeData.micSupport)
        {
            *pEvaluation = 2;
        }
        else if (aironetIeData.kpSupport && !aironetIeData.micSupport)
        {
            *pEvaluation = 3;
        }
        else if (!aironetIeData.kpSupport && aironetIeData.micSupport)
        {
            *pEvaluation = 4;
        }
        else if (aironetIeData.kpSupport && aironetIeData.micSupport)
        {
            *pEvaluation = 5;
        }
    }

    /* Check privacy bit if not in mixed mode */
    if (!pAdmCtrl->mixedMode)
    {   /* There's no mixed mode, so make sure that the privacy Bit matches the privacy mode*/
        if (!pRsnData->privacy)
            {
                *pEvaluation = 0;
                return TI_NOK;
            }
    }

    return TI_OK;
}



/**
*
* admCtrlCcx_parseAironetIe  - Parse an Aironet information element.
*
* \b Description: 
*
* Parse an Aironet information element. 
* Builds a structure of all the capabilities described in the Aironet IE.
* We look at Flags field only to determine KP and MIC bits value
*
* \b ARGS:
*
*  I   - pAdmCtrl - pointer to admCtrl context
*  I   - pAironetIe - pointer to Aironet IE buffer  \n
*  O   - pAironetData - capabilities structure
*  
*  
* \b RETURNS:
*
* TI_OK on success, TI_NOK on failure. 
*
* \sa 
*/
TI_STATUS admCtrlCcx_parseAironetIe(admCtrl_t *pAdmCtrl, TI_UINT8 *pAironetIe, aironetIeData_t *pAironetData)
{

    aironetIePacket_t   *aironetIePacket = (aironetIePacket_t*)pAironetIe;
     

    if (pAironetData == NULL) 
    {
        return TI_NOK;
    }

    if ((pAironetIe == NULL) || (aironetIePacket->length < RSN_AIRONET_FLAGS_OFFSET) ||
        (aironetIePacket->elementid != CCX_EXT_1_IE_ID) )
    {
        os_memoryZero(pAdmCtrl->hOs, pAironetData, sizeof(aironetIeData_t));
        return TI_OK; 
    }

    TRACE1(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "Aironet_IE: DEBUG: Flags:%x \n", aironetIePacket->Flags);

    /* Parse values*/
    if (aironetIePacket->Flags  & AIRONET_MIC_ENABLED)
    {
        pAironetData->micSupport = 1;
    }
    if (aironetIePacket->Flags  & AIRONET_KP_ENABLED)
    {
        pAironetData->kpSupport = 1;
    }
    if (aironetIePacket->Flags  & DOT11_AIRO_PROXY_ARP)
    {
        pAironetData->proxyArpEnabled = 1;
    }
    pAironetData->refreshRate = aironetIePacket->refresh_rate; /* keep-alive interval, for CCX v4 */
    os_memoryCopy(pAdmCtrl->hOs, pAironetData->reserved, &aironetIePacket->load, 8);
    
    TRACE10(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "Aironet_IE: DEBUG: Capabilities: MIC %d, KP:%d , reserved = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", pAironetData->micSupport, pAironetData->kpSupport, pAironetData->reserved[0], pAironetData->reserved[1], pAironetData->reserved[2], pAironetData->reserved[3], pAironetData->reserved[4], pAironetData->reserved[5], pAironetData->reserved[6], pAironetData->reserved[7]);

    return TI_OK;

}




/**
*
* admCtrlCcx_parseAironetIe  - Parse an Aironet information element.
*
* \b Description: 
*
* Parse an Aironet information element. 
* Builds a structure of all the capabilities described in the Aironet IE.
* We look at Flags field only to determine KP and MIC bits value
*
* \b ARGS:
*
*  I   - pAdmCtrl - pointer to admCtrl context
*  I   - pApIptIe - pointer to AP IP IE buffer  \n
*  O   - pApIpData - AP IP Address
*  
*  
* \b RETURNS:
*
* TI_OK on success, TI_NOK on failure. 
*
* \sa 
*/
TI_STATUS admCtrlCcx_parseApIpAddrIe(admCtrl_t *pAdmCtrl, TI_UINT8 *pApIptIe, TI_UINT8 *pApIpData)
{

    apIpAddrIePacket_t  *pApIpAddrIePacket = (apIpAddrIePacket_t*)pApIptIe;
     
    TRACE0(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "AP_IP_IE: DEBUG: admCtrlCcx_parseApIpAddrIe\n\n");

    if (pApIpData == NULL) 
    {
        return TI_NOK;
    }

    if ((pApIptIe == NULL) || (pApIpAddrIePacket->length < RSN_AP_IP_ADDRESS_OFFSET) ||
        (pApIpAddrIePacket->elementid != CCX_EXT_2_IE_ID) )
    {
        os_memoryZero(pAdmCtrl->hOs, pApIpData, 4);
        return TI_OK; 
    }

    TRACE4(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "AP_IP_IE: DEBUG: Address:%x, %x, %x, %x \n", pApIpAddrIePacket->IpAdress[0], pApIpAddrIePacket->IpAdress[1], pApIpAddrIePacket->IpAdress[2], pApIpAddrIePacket->IpAdress[3]);

    os_memoryCopy(pAdmCtrl->hOs, pApIpData, pApIpAddrIePacket->IpAdress, 4);
    
    return TI_OK;

}

/**
*
* admCtrlCcx_parseCckmSuiteVal  - Parse a WPA information element.
*
* \b Description: 
*
* Parse a WPA information element. 
* Looks for CCKM Authenticated Key Management Suite.
*
* \b ARGS:
*
*  I   - pAdmCtrl - pointer to admCtrl context
*  I   - suiteVal - pointer to WPA IE with the AKM \n
*  
*  
* \b RETURNS:
*
* The AKM - CCKM or NA. 
*
* \sa 
*/
TI_UINT32  admCtrlCcx_parseCckmSuiteVal(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal)
{
    TI_UINT32          suite;
    static TI_UINT8    wpaIeCcxOuiIe[3] = CCX_OUI;

    if (suiteVal==NULL)
    {
        return TWD_CIPHER_UNKNOWN;
    }
    if (!os_memoryCompare(pAdmCtrl->hOs, suiteVal, wpaIeCcxOuiIe, 3))
    {
        suite =  (ECipherSuite)((suiteVal[3]==CCX_IE_KEY_MNG_CCKM) ? WPA_IE_KEY_MNG_CCKM : WPA_IE_KEY_MNG_NA); 
        if (suite == WPA_IE_KEY_MNG_CCKM)
            TRACE0(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "CCKM AKM is found \n\n");
    } else
    {
        suite = WPA_IE_KEY_MNG_NA;
    }
    return  suite;

}

/**
*
* admCtrlCcx_parseCckmSuiteVal  - Parse a WPA information element.
*
* \b Description: 
*
* Parse a WPA information element. 
* Looks for CCKM Authenticated Key Management Suite.
*
* \b ARGS:
*
*  I   - pAdmCtrl - pointer to admCtrl context
*  I   - suiteVal - pointer to WPA IE with the AKM \n
*  
*  
* \b RETURNS:
*
* The AKM - CCKM or NA. 
*
* \sa 
*/
TI_UINT32  admCtrlCcx_parseCckmSuiteVal4Wpa2(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal)
{
    TI_UINT32          suite;
    static TI_UINT8    wpaIeCcxOuiIe[3] = CCX_OUI;

    if (suiteVal==NULL)
    {
        return TWD_CIPHER_UNKNOWN;
    }
    if (!os_memoryCompare(pAdmCtrl->hOs, suiteVal, wpaIeCcxOuiIe, 3))
    {
        suite =  (ECipherSuite)((suiteVal[3]==CCX_IE_KEY_MNG_CCKM) ? WPA2_IE_KEY_MNG_CCKM : WPA2_IE_KEY_MNG_NA); 
        if (suite == WPA2_IE_KEY_MNG_CCKM)
            TRACE0(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "CCKM AKM is found \n\n");
    } else
    {
        suite = WPA2_IE_KEY_MNG_NA;
    }
    return  suite;

}

/**
*
* admCtrlCcx_getCckmAkm  - Gets the CCKM AKM if exists.
*
* \b Description: 
*
* Gets the CCKM AKM if exists. 
* Looks for CCKM Authenticated Key Management Suite.
*
* \b ARGS:
*
*  I   - pAdmCtrl - pointer to admCtrl context
*  O   - akmSuite - pointer to the returned AKM \n
*  
*  
* \b RETURNS:
*
* The AKM - CCKM or NA. 
*
* \sa 
*/
TI_BOOL  admCtrlCcx_getCckmAkm(admCtrl_t *pAdmCtrl, TI_UINT8 *akmSuite)
{
    static TI_UINT8    wpaIeCcxOuiIe[3] = CCX_OUI;
    paramInfo_t     param;
    TI_STATUS       status;


    if ((pAdmCtrl==NULL) || (akmSuite==NULL))
    {
        return TI_FALSE;
    }
    param.paramType = CCX_CCKM_EXISTS;
    status = ccxMngr_getParam(pAdmCtrl->hCcxMngr, &param);
    if (status!=TI_OK)
    {
        return TI_FALSE;
    }
    if (param.content.ccxCckmExists)
    {   /* The OUI is different in CCKM case */
        os_memoryCopy(pAdmCtrl->hOs, akmSuite, wpaIeCcxOuiIe, 3);
        akmSuite[3] = CCX_IE_KEY_MNG_CCKM;
        TRACE0(pAdmCtrl->hReport, REPORT_SEVERITY_INFORMATION, "CCKM AKM exists \n\n");
        return TI_TRUE;

    }

   return TI_FALSE;

}


/**
*
* admCtrlCcx_setExtendedParams  - Set current primary Aironet parameters.
*
* \b Description: 
*
* Set current primary Aironet parameters.
*
* \b ARGS:
*
*  I   - pAdmCtrl - context \n
*  I   - pRsnData - site's RSN data \n
*  
* \b RETURNS:
*
*  TI_OK on site is aproved, TI_NOK on site is rejected.
*
* \sa 
*/
TI_STATUS admCtrlCcx_setExtendedParams(admCtrl_t *pAdmCtrl, TRsnData *pRsnData)
{
    TI_STATUS           status;
    aironetIeData_t     aironetIeData;
    TI_UINT8            ApIpAdress[4];
    TI_UINT8            *pAironetIe=NULL;
    TI_UINT8            *pApIpAddrData=NULL;

    if (pRsnData==NULL)
    {
        return TI_NOK;
    }
    os_memoryZero(pAdmCtrl->hOs, &aironetIeData, sizeof(aironetIeData));

    status = admCtrl_parseIe(pAdmCtrl, pRsnData, &pAironetIe, CCX_EXT_1_IE_ID);
    if (status != TI_OK)                                                         
    {                                                                                    
        return status;                                                        
    }
    if (pAironetIe!=NULL)
    {   
        pAdmCtrl->ccxSupport = TI_TRUE;
        status = admCtrl_parseIe(pAdmCtrl, pRsnData, &pApIpAddrData, CCX_EXT_2_IE_ID);
        if (status != TI_OK)
        {
            return status;
        }
        
        status = admCtrlCcx_parseAironetIe(pAdmCtrl, pAironetIe, &aironetIeData);
        if (status != TI_OK)
        {
            return status;
        }
        
        status = admCtrlCcx_parseApIpAddrIe(pAdmCtrl, pApIpAddrData, ApIpAdress);
        if (status==TI_OK)
        {
           os_memoryCopy(pAdmCtrl->hOs ,pAdmCtrl->AP_IP_Address, ApIpAdress, 4);    
        }
        else
        {
            return status;
        }
        os_memoryCopy(pAdmCtrl->hOs, pAdmCtrl->aironetIeReserved, aironetIeData.reserved, 8);

        pAdmCtrl->proxyArpEnabled = (aironetIeData.proxyArpEnabled==1) ? TI_TRUE: TI_FALSE;

    }
    else
    {   /* WEP */
        pAdmCtrl->ccxSupport = TI_FALSE;
    }

    return status;
}


TI_UINT32  admCtrlCcx_WpaParseSuiteVal(admCtrl_t *pAdmCtrl, TI_UINT8* suiteVal, wpaIeData_t *pWpaData)
{
#ifdef CKIP_ENABLED    
    TI_UINT32  suite;
    TI_UINT8   ccx_oui[] = CCX_OUI;
#endif

    if ((pAdmCtrl==NULL) || (suiteVal==NULL))
    {
        return TWD_CIPHER_UNKNOWN;
    }

#ifndef CKIP_ENABLED    
   /* If CKIP is not meant to be supported, return CIPHER_UNKNOWN which will prevent us from connecting */
   TRACE0(pAdmCtrl->hReport , REPORT_SEVERITY_ERROR, "Not supporting CKIP! returning !!!\n");
        return TWD_CIPHER_UNKNOWN;
#else
    if (!os_memoryCompare(pAdmCtrl->hOs, suiteVal, ccx_oui, 3))
    {
        suite =  (ECipherSuite)((suiteVal[3]<=CMIC_ONLY) ? suiteVal[3] : CIPHER_UNKNOWN); 
    } else
    {
        suite = CIPHER_UNKNOWN;
    }
    
    switch(suite)
    {
    case CKIP_ONLY:
        pWpaData->ccxKp = TI_TRUE;
        pWpaData->ccxMic = TI_FALSE;
        suite = CIPHER_CKIP;
        break;
    case CKIP_CMIC:
        pWpaData->ccxKp = TI_TRUE;
        pWpaData->ccxMic = TI_TRUE;
        suite = CIPHER_CKIP;
        break;
    case CMIC_ONLY:
        pWpaData->ccxKp = TI_FALSE;
        pWpaData->ccxMic = TI_TRUE;
        suite = CIPHER_CKIP;
        break;
    default:
        suite = CIPHER_UNKNOWN; 
        pWpaData->ccxKp = TI_FALSE;
        pWpaData->ccxMic = TI_FALSE;
    }
    
    return  suite;
#endif
}


TI_STATUS admCtrlCcx_getWpaCipherInfo(admCtrl_t *pAdmCtrl, wpaIePacket_t *pWpaIePacket)
{

#ifndef CKIP_ENABLED    /* In case ckip is not enabled we do not send the AiroNet InfoElement.*/     
        return TI_OK;
#else

        static TI_UINT8    wpaIeCcxOuiAndMaxSecurity[4] = {0x00,0x40,0x96,0x01};
        static TI_UINT8    wpaIeCcxOuiAndCkipOnly[4] = {0x00,0x40,0x96,0x00};
        static TI_UINT8    wpaIeCcxOuiAndCmicOnly[4] = {0x00,0x40,0x96,0x02};

        /* build pairwise suite */
#ifndef TIWLN_WINCE30        
        pWpaIePacket->pairwiseSuiteCnt = ENDIAN_HANDLE_WORD(0x0001);
#else
        /* required for WinCe, when the pointer is not even */
        tempInt =  ENDIAN_HANDLE_WORD(0x0001);
        os_memoryCopy (pAdmCtrl->hOs,(TI_UINT8*)&pWpaIePacket->pairwiseSuiteCnt, &tempInt, sizeof(pWpaIePacket->pairwiseSuiteCnt));
#endif
        if (pAdmCtrl->encrInSw && pAdmCtrl->micInSw)
        {
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->pairwiseSuite, wpaIeCcxOuiAndMaxSecurity, 4);
            /* build group suite */
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->groupSuite, wpaIeCcxOuiAndMaxSecurity, 4);
        }
        else if (pAdmCtrl->encrInSw && !pAdmCtrl->micInSw)
        {
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->pairwiseSuite, wpaIeCcxOuiAndCkipOnly, 4);
            /* build group suite */
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->groupSuite, wpaIeCcxOuiAndCkipOnly, 4);
        }
        else
        {
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->pairwiseSuite, wpaIeCcxOuiAndCmicOnly, 4);
            /* build group suite */
            os_memoryCopy(pAdmCtrl->hOs, (void*)pWpaIePacket->groupSuite, wpaIeCcxOuiAndCmicOnly, 4);
        }

    return TI_OK;
#endif
}

#endif /*CCX_MODULE_INCLUDED*/



