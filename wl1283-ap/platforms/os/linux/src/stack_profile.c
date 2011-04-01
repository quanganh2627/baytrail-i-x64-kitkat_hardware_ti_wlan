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
 * src/stack_profile.c
 *
 */

#include <linux/stddef.h>

unsigned long check_stack(unsigned long *base)
{
	
	register unsigned long sp asm ("sp");
	unsigned long retval = sp;
	*base = ((sp & ~0x1fff) + 0x380);
	return retval;
}

unsigned long check_stack_start(unsigned long *base)
{
    unsigned long i;
	unsigned long from,to;

	to = check_stack(&from);
	*base = from;

	/* run from the stack pointer down to the base */
	for (i = from;i<to;i+=4) 
	{
		/* fill up the pattern */
		*(long *)i = 0xdeadbeef;
	}
/*	printk("check_stack_start: from =%x to=%x data=%x\n",from,to,*(long *)(from+4));*/
	return to;
}

unsigned long check_stack_stop(unsigned long *base)
{
    unsigned long i;
	unsigned long from,to;

	to = check_stack(&from);
	*base = from;

	/* run from the stack pointer down to the base */
	for (i = from;i<to;i+=4) 
	{
		/* check up the pattern */
		if ((*(long *)i) != 0xdeadbeef)
			break;
	}

	/*printk("check_stack_stop: from =%x to=%x data=%x data=%x i=0x%x\n",from,to,*(long *)from,*(long *)(from+4),i);*/
	/* return the first time when the pattern doesn't match */
	return i;
}


