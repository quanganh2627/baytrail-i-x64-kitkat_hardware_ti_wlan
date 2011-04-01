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
/*																		   */
/*	  MODULE:	wbuf.h												       */
/*    PURPOSE:	manages the allocation/free and field access of the WBUF   */
/*																		   */
/***************************************************************************/
#ifndef _WBUF_H_
#define _WBUF_H_

#include <linux/version.h>
#include <linux/skbuff.h>
#include "tidef.h"
#include "queue.h"

typedef void WBUF;

/* Packet types */
typedef enum
{
	TX_PKT_TYPE_MGMT        = 0x01,
	TX_PKT_TYPE_EAPOL       = 0x02,
	TX_PKT_TYPE_ETHER       = 0x04,	/* Data packets from the Network interface. */
	TX_PKT_TYPE_WLAN_DATA	= 0x08 	/* Currently includes Null and IAPP packets. */
} TX_PKT_TYPE;

#define TX_PKT_FLAGS_LINK_TEST		0x1   /* Tx-Packet-Flag */
#define WSPI_PAD_BYTES              16    /* Add padding before data buffer for WSPI overhead */

/* user callback definition (in tx complete) */
typedef void *CB_ARG;
typedef void (*CB_FUNC)(CB_ARG cb_arg);

/*
 * wbuf user fields: 
 */
typedef struct 
{
    TQueNodeHdr     queNodeHdr; /* The header used for queueing the WBUF            */
	TX_PKT_TYPE		pktType;	/* wbuf packet type									*/
	CB_FUNC			cb_func;	/* callback function to use in tx complete			*/
	CB_ARG			cb_arg;		/* callback argument to use in tx complete			*/
    TI_UINT8		Tid;		/* WLAN TID (traffic identity)						*/
    TI_UINT8		hdrLen;		/* WLAN header length, not including alignment pad. */
    TI_UINT8		flags;		/* Some application flags, see Tx-Packet-Flags defs above. */
} WBUF_PARAMS;



#define WBUF_DATA(pWbuf)			( ((struct sk_buff *)(pWbuf))->data )
#define WBUF_LEN(pWbuf)				( ((struct sk_buff *)(pWbuf))->len )
#define WBUF_PRIORITY(pWbuf)		( ((struct sk_buff *)(pWbuf))->priority )
#define WBUF_DEV(pWbuf)				( ((struct sk_buff *)(pWbuf))->dev )
#define WBUF_DEV_SET(pWbuf,pDev)    ( ((struct sk_buff *)(pWbuf))->dev) = ((struct net_device *)(pDev))
#ifdef  HOST_PLATFORM_OMAP3430 || HOST_PLATFORM_ZOOM2 || HOST_PLATFORM_ZOOM1
#define WBUF_STAMP(pWbuf)			( ((struct sk_buff *)(pWbuf))->tstamp.tv.nsec )
#else
#define WBUF_STAMP(pWbuf)			( ((struct sk_buff *)(pWbuf))->tstamp.off_usec )
#endif
#define WBUF_CB(pWbuf)				( ((struct sk_buff *)(pWbuf))->cb )
#define WBUF_PKT_TYPE(pWbuf)		( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->pktType )
#define WBUF_CB_FUNC(pWbuf)		    ( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->cb_func )
#define WBUF_CB_ARG(pWbuf)		    ( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->cb_arg )
#define WBUF_TID(pWbuf)		        ( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->Tid )
#define WBUF_HDR_LEN(pWbuf)			( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->hdrLen )
#define WBUF_FLAGS(pWbuf)			( ((WBUF_PARAMS *)&(WBUF_CB(pWbuf)))->flags )

/* The offset of the node-header field from the WBUF entry (for queueing) */
#define WBUF_NODE_HDR_OFFSET    \
		( (unsigned long) &( ( (WBUF_PARAMS *) &( ( (struct sk_buff *)0 )->cb ) )->queNodeHdr ) )

/* 
 * Allocate WBUF for Tx/Rx packets.
 * Add 16 bytes before the data buffer for WSPI overhead!
 */
static inline WBUF *WbufAlloc (TI_HANDLE hOs, TI_UINT32 len)
{
     WBUF *pWbuf = alloc_skb (len + WSPI_PAD_BYTES, GFP_ATOMIC);
     WBUF_DATA (pWbuf) += WSPI_PAD_BYTES;
     return pWbuf;
}

#define WbufFree(hOs, pWbuf)		( dev_kfree_skb((struct sk_buff *)pWbuf) )
#define WbufReserve(hOs, pWbuf,len)	( skb_reserve((struct sk_buff *)pWbuf,(int)len) )

#endif
