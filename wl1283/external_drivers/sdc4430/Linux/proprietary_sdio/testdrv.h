/*
 * testdrv.h
 *
 * Copyright(c) 1998 - 2010 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _MMC_TEST_H_
#define _MMC_TEST_H_

#define TESTDRV_MODULE_NAME "sdio_test"

#ifdef  TESTDRV_CONFIG_MMC_TEST_DEBUG
#define DBG(x...)	printk(x)
#else
#define DBG(x...)	do { } while (0)
#endif

#define TESTDRV_GPIO_OUTPUT		                0

#if 0
#include <asm/arch/hardware.h>
#endif
#include <mach/hardware.h>

#define TXN_FUNC_ID_CTRL         0
#define TXN_FUNC_ID_BT           1
#define TXN_FUNC_ID_WLAN         2

/* in case of defined in include/asm/arch/omap34xx.h */
#ifndef OMAP_CTRL_BASE
#define OMAP_CTRL_BASE          		OMAP343X_CTRL_BASE
#endif
#define OMAP_HSMMC2_BASE				0x480b4000


#define CONTROL_PADCONF_MMC2_CLK        0x48002158   /* mmc2_cmd */
#define CONTROL_PADCONF_MMC2_DAT0       0x4800215C   /* mmc2_dat0, mmc2_dat1 */
#define CONTROL_PADCONF_MMC2_DAT2       0x48002160   /* mmc2_dat2, mmc2_dat3 */

#ifdef EXPANSION_BOARD_MCS7
#define CONTROL_PADCONF_SYS_BOOT1       0x48002164   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x48002168   /* WLAN_IRQ    */
#define PMENA_GPIO                      136
#define IRQ_GPIO                        139
#else 
#ifdef EXPANSION_BOARD_TST
#define CONTROL_PADCONF_SYS_BOOT1       0x48002A0C   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x480020C8   /* WLAN_IRQ    */
#define PMENA_GPIO                      3
#define IRQ_GPIO                        61
#else
#define CONTROL_PADCONF_GPMC_NCS5       0x480020B8   /* WLAN_ENABLE */
#define CONTROL_PADCONF_GPMC_NBE1       0x480020C8   /* WLAN_IRQ    */
#define PMENA_GPIO                      57
#define IRQ_GPIO                        61
#endif
#endif


#define MUXMODE_3                       3
#define TNETW_IRQ                       (OMAP_GPIO_IRQ(IRQ_GPIO))
#define TIWLAN_IRQ_POLL_INTERVAL	    HZ/100
#define HZ_IN_MSEC						HZ/1000
#define TIWLAN_IRQ_POLL_INTERVAL_MS		TIWLAN_IRQ_POLL_INTERVAL/HZ_IN_MSEC

#define TESTDRV_SDIO_FUNC1_OFFSET           	0x1FFC0  /* address of the partition table */

#define SDIO_TEST_FIRST_VALID_DMA_ADDR			(0x00000008)	/* used for escaping addressing invalid DMA Addresses */
#define SDIO_TEST_NO_OF_TRANSACTIONS			(3)

#define TESTDRV_512_SDIO_BLOCK					(512)
#define TESTDRV_MAX_SDIO_BLOCK					(TESTDRV_512_SDIO_BLOCK /* - 4 */)

#define TESTDRV_MAX_PART_SIZE  					0x1F000 	/* 124k	*/	 

#define TESTDRV_CODE_RAM_SIZE  					0x30000		/* 192K	*/
#define TESTDRV_DATA_RAM_SIZE 					0xC000		/* 48K 	*/
#define TESTDRV_PACKET_RAM_SIZE 				0xD000		/* 52K 	*/

#define TESTDRV_REG_PART_START_ADDR 			0x300000
#define TESTDRV_REG_DOWNLOAD_PART_SIZE 			0x8800 		/* 44k	*/ 	
#define TESTDRV_REG_WORKING_PART_SIZE 			0xB000 		/* 44k	*/ 	

#define TESTDRV_CODE_RAM_PART_START_ADDR 		0		
#define TESTDRV_DATA_RAM_PART_START_ADDR 		0x20000000
#define TESTDRV_PACKET_RAM_PART_START_ADDR 		0x40000

/* Partition Size Left for Memory */
#define TESTDRV_MEM_WORKING_PART_SIZE  			(TESTDRV_MAX_PART_SIZE - TESTDRV_REG_WORKING_PART_SIZE) 	 
#define TESTDRV_MEM_DOWNLOAD_PART_SIZE  		(TESTDRV_MAX_PART_SIZE - TESTDRV_REG_DOWNLOAD_PART_SIZE) 	 

#define TESTDRV_TESTING_DATA_LENGTH 512
#endif /* _MMC_TEST_H_ */

