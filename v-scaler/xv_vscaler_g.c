/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "vprocss_hw_config.h"
#include "xv_vscaler.h"

/*
* The configuration table for devices
*/

XV_vscaler_Config XV_vscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_VSCALER_NUM_INSTANCES
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_DEVICE_ID,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_S_AXI_CTRL_BASEADDR,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_SAMPLES_PER_CLOCK,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_NUM_VIDEO_COMPONENTS,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_MAX_COLS,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_MAX_ROWS,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_MAX_DATA_WIDTH,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_PHASE_SHIFT,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_SCALE_MODE,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_TAPS,
		XPAR_HDMI_PATH_V_PROC_SS_0_V_VSCALER_ENABLE_420
#endif
	}
};
