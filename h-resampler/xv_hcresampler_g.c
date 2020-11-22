/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "vprocss_hw_config.h"
#include "xv_hcresampler.h"

/*
* The configuration table for devices
*/

XV_hcresampler_Config XV_hcresampler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_DEVICE_ID,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_S_AXI_CTRL_BASEADDR,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_SAMPLES_PER_CLOCK,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_MAX_COLS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_MAX_ROWS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_MAX_DATA_WIDTH,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_CONVERT_TYPE,
		XPAR_HDMI_PATH_V_PROC_SS_0_HCR_NUM_H_TAPS
#endif
	}
};
