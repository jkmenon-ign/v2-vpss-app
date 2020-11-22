// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

#include "vprocss_hw_config.h"
#include "xv_hcresampler.h"

#ifndef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_HCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_hcresampler_Config XV_hcresampler_ConfigTable[];

XV_hcresampler_Config *XV_hcresampler_LookupConfig(u16 DeviceId) {
    XV_hcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_HCRESAMPLER_NUM_INSTANCES; Index++) {
        if (XV_hcresampler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_hcresampler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}



#endif
