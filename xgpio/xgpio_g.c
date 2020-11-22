/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xgpio_g.c
* @addtogroup gpio_v4_6
* @{
*
* This file contains a configuration table that specifies the configuration
* of GPIO devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  02/04/02 First release
* 2.00a jhl  12/16/02 Update for dual channel and interrupt support
* 2.11a mta  03/21/07 Updated to new coding style
* 4.0   sha  07/15/15 Added XPAR_XGPIO_NUM_INSTANCES macro to control
*		      config table parameters.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xgpio.h"
#include "vprocss_hw_config.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each GPIO device
 * in the system.
 */
XGpio_Config XGpio_ConfigTable[] = {
	{
#ifdef XPAR_XGPIO_NUM_INSTANCES
	 XPAR_HDMI_PATH_V_PROC_SS_0_RESET_SEL_AXIS_DEVICE_ID,
	 XPAR_HDMI_PATH_V_PROC_SS_0_RESET_SEL_AXIS_BASEADDR,
	 XPAR_HDMI_PATH_V_PROC_SS_0_RESET_SEL_AXIS_INTERRUPT_PRESENT,
	 XPAR_HDMI_PATH_V_PROC_SS_0_RESET_SEL_AXIS_IS_DUAL
#endif
	}
};
/** @} */
