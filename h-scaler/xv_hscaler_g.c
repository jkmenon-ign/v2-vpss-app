/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "vprocss_hw_config.h"
#include "xv_hscaler.h"

/*
* The configuration table for devices
*/

XV_hscaler_Config XV_hscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HSCALER_NUM_INSTANCES
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_DEVICE_ID,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_S_AXI_CTRL_BASEADDR,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_SAMPLES_PER_CLOCK,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_NUM_VIDEO_COMPONENTS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_MAX_COLS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_MAX_ROWS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_MAX_DATA_WIDTH,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_PHASE_SHIFT,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_SCALE_MODE,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_TAPS,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_ENABLE_422,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_ENABLE_420,
		XPAR_HDMI_PATH_V_PROC_SS_0_HSC_ENABLE_CSC
#endif
	}
};
