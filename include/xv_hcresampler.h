// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_HCRESAMPLER_H
#define XV_HCRESAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#include "xv_hcresampler_hw.h"
#include "common.h"


/**************************** Type Definitions ******************************/
#ifdef __linux__

/**
* This typedef contains configuration information for the horziontal
* chroma resampler core. Each core instance should have a configuration
* structure associated.
*/
typedef struct {
    u16 DeviceId;          /**< Unique ID  of device */
    UINTPTR BaseAddress;   /**< The base address of the core instance. */
    int PixPerClk;         /**< Samples Per Clock supported by core instance */
    u16 MaxWidth;          /**< Maximum columns supported by core instance */
    u16 MaxHeight;         /**< Maximum rows supported by core instance */
    int MaxDataWidth;      /**< Maximum Data width of each channel */
    int ResamplingType;    /**< Resampling Method selected */
    u8  NumTaps;           /**< Number of filter taps */
} XV_hcresampler_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_hcresampler_Config Config; /**< Hardware Configuration */
    u32 IsReady;                  /**< Device is initialized and ready */
} XV_hcresampler;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes *****************************/
#ifdef __linux__
int XV_hcresampler_Initialize(XV_hcresampler *InstancePtr, u16 DeviceId);
XV_hcresampler_Config* XV_hcresampler_LookupConfig(u16 DeviceId);
int XV_hcresampler_CfgInitialize(XV_hcresampler *InstancePtr,
                                 XV_hcresampler_Config *ConfigPtr,
								 UINTPTR EffectiveAddr);
//int XV_hcresampler_Initialize(XV_hcresampler *InstancePtr, const char* InstanceName);
//int XV_hcresampler_Release(XV_hcresampler *InstancePtr);
#endif

void XV_hcresampler_Start(XV_hcresampler *InstancePtr);
u32 XV_hcresampler_IsDone(XV_hcresampler *InstancePtr);
u32 XV_hcresampler_IsIdle(XV_hcresampler *InstancePtr);
u32 XV_hcresampler_IsReady(XV_hcresampler *InstancePtr);
void XV_hcresampler_EnableAutoRestart(XV_hcresampler *InstancePtr);
void XV_hcresampler_DisableAutoRestart(XV_hcresampler *InstancePtr);

void XV_hcresampler_Set_HwReg_width(XV_hcresampler *InstancePtr, u32 Data);
u32 XV_hcresampler_Get_HwReg_width(XV_hcresampler *InstancePtr);
void XV_hcresampler_Set_HwReg_height(XV_hcresampler *InstancePtr, u32 Data);
u32 XV_hcresampler_Get_HwReg_height(XV_hcresampler *InstancePtr);
void XV_hcresampler_Set_HwReg_input_video_format(XV_hcresampler *InstancePtr, u32 Data);
u32 XV_hcresampler_Get_HwReg_input_video_format(XV_hcresampler *InstancePtr);
void XV_hcresampler_Set_HwReg_output_video_format(XV_hcresampler *InstancePtr, u32 Data);
u32 XV_hcresampler_Get_HwReg_output_video_format(XV_hcresampler *InstancePtr);
void XV_hcresampler_Set_HwReg_coefs_0_0(XV_hcresampler *InstancePtr, u32 Data);
u32 XV_hcresampler_Get_HwReg_coefs_0_0(XV_hcresampler *InstancePtr);

void XV_hcresampler_InterruptGlobalEnable(XV_hcresampler *InstancePtr);
void XV_hcresampler_InterruptGlobalDisable(XV_hcresampler *InstancePtr);
void XV_hcresampler_InterruptEnable(XV_hcresampler *InstancePtr, u32 Mask);
void XV_hcresampler_InterruptDisable(XV_hcresampler *InstancePtr, u32 Mask);
void XV_hcresampler_InterruptClear(XV_hcresampler *InstancePtr, u32 Mask);
u32 XV_hcresampler_InterruptGetEnabled(XV_hcresampler *InstancePtr);
u32 XV_hcresampler_InterruptGetStatus(XV_hcresampler *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
