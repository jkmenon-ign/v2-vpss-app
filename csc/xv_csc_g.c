/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "vprocss_hw_config.h"
#include "xv_csc.h"

/*
* The configuration table for devices
*/

XV_csc_Config XV_csc_ConfigTable[] =
{
	{
#ifdef XPAR_XV_CSC_NUM_INSTANCES
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_DEVICE_ID,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_S_AXI_CTRL_BASEADDR,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_SAMPLES_PER_CLOCK,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_V_CSC_MAX_WIDTH,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_V_CSC_MAX_HEIGHT,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_MAX_DATA_WIDTH,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_ENABLE_422,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_ENABLE_420,
		XPAR_HDMI_PATH_V_PROC_SS_0_CSC_ENABLE_WINDOW
#endif
	}
};
