// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

#include "vprocss_hw_config.h"
#include "xv_vcresampler.h"

#ifndef XPAR_XV_VCRESAMPLER_NUM_INSTANCES
#define XPAR_XV_VCRESAMPLER_NUM_INSTANCES   0
#endif

extern XV_vcresampler_Config XV_vcresampler_ConfigTable[];

XV_vcresampler_Config *XV_vcresampler_LookupConfig(u16 DeviceId) {
    XV_vcresampler_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_VCRESAMPLER_NUM_INSTANCES; Index++) {
        if (XV_vcresampler_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_vcresampler_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}



#endif
