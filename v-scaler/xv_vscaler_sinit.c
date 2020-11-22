// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

#include "vprocss_hw_config.h"
#include "xv_vscaler.h"

#ifndef XPAR_XV_VSCALER_NUM_INSTANCES
#define XPAR_XV_VSCALER_NUM_INSTANCES   0
#endif

extern XV_vscaler_Config XV_vscaler_ConfigTable[];

XV_vscaler_Config *XV_vscaler_LookupConfig(u16 DeviceId) {
    XV_vscaler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_VSCALER_NUM_INSTANCES; Index++) {
        if (XV_vscaler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_vscaler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}



#endif
