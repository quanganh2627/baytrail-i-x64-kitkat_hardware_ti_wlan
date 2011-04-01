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
 * inc/ioctl_init.h
 *
 */

#ifndef TI1610_IOCTL_INIT
#define TI1610_IOCTL_INIT

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>

#ifndef DRIVER_NAME
#define DRIVER_NAME "TIWLAN"
#endif

#if defined(DEBUG_MESSAGES)
#  define print_deb(fmt, arg...) printk(KERN_INFO DRIVER_NAME ": " fmt,##arg)
#else
#  define print_deb(fmt, arg...) 
#endif

#if defined(OS_INFO_MESSAGES)
#  define print_info(fmt, arg...) printk(KERN_INFO DRIVER_NAME ": " fmt,##arg)
#else
#  define print_info(fmt, arg...)
#endif

#ifndef print_err
#  define print_err(fmt, arg...) printk(KERN_ERR DRIVER_NAME ": " fmt,##arg)
#endif


#endif /* TI1610_IOCTL_INIT */
