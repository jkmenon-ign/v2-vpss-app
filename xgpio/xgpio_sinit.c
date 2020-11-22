/******************************************************************************
* Copyright (C) 2003 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_sinit.c
* @addtogroup gpio_v4_6
* @{
*
* The implementation of the XGpio driver's static initialization
* functionality.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 2.01a jvb  10/13/05 First release
* 2.11a mta  03/21/07 Updated to new coding style
* 4.0   sha  07/15/15 Defined macro XPAR_XGPIO_NUM_INSTANCES if not
*		      defined in xparameters.h
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "vprocss_hw_config.h"
#include "xgpio_i.h"

/************************** Constant Definitions ****************************/

#ifndef XPAR_XGPIO_NUM_INSTANCES
#define XPAR_XGPIO_NUM_INSTANCES		0
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/******************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param	DeviceId is the device identifier to lookup.
*
* @return
*		 - A pointer of data type XGpio_Config which points to the
*		device configuration if DeviceID is found.
* 		- NULL if DeviceID is not found.
*
* @note		None.
*
******************************************************************************/
XGpio_Config *XGpio_LookupConfig(u16 DeviceId)
{
	XGpio_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XGPIO_NUM_INSTANCES; Index++) {
		if (XGpio_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XGpio_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}



/** @} */
