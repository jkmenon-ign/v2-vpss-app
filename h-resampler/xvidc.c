/*******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvidc.c
 * @addtogroup video_common_v4_9
 * @{
 *
 * Contains common utility functions that are typically used by video-related
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   rc,  01/10/15 Initial release.
 *       als
 * 2.2   als  02/01/16 Functions with pointer arguments that don't modify
 *                     contents now const.
 *                     Added ability to insert a custom video timing table.
 *       yh            Added 3D support.
 * 3.0   aad  05/13/16 Added API to search for RB video modes.
 * 3.1   rco  07/26/16 Added extern definition for timing table array
 *                     Added video-in-memory color formats
 *                     Updated XVidC_RegisterCustomTimingModes API signature
 * 4.1   rco  11/23/16 Added new memory formats
 *                     Added new API to get video mode id that matches exactly
 *                     with provided timing information
 *                     Fix c++ warnings
 * 4.2	 jsr  07/22/17 Added new framerates and color formats to support SDI
 *                     Reordered YCBCR422 colorforamt and removed other formats
 *                     that are not needed for SDI which were added earlier.
 *       vyc  10/04/17 Added new streaming alpha formats and new memory formats
 * 4.3   eb   26/01/18 Added API XVidC_GetVideoModeIdExtensive
 *       jsr  02/22/18 Added XVIDC_CSF_YCBCR_420 color space format
 *       vyc  04/04/18 Added BGR8 memory format
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvidc.h"



/**************************** Function Prototypes *****************************/

/**
 * This function returns the color format name for index specified.
 *
 * @param	ColorFormatId specifies the index of color format space.
 *
 * @return	Pointer to a color space name string.
 *
 * @note	None.
 *
*******************************************************************************/
const char *XVidC_GetColorFormatStr(XVidC_ColorFormat ColorFormatId)
{
	switch (ColorFormatId) {
		case XVIDC_CSF_RGB:            return ("RGB");
		case XVIDC_CSF_YCRCB_444:      return ("YUV_444");
		case XVIDC_CSF_YCRCB_422:      return ("YUV_422");
		case XVIDC_CSF_YCRCB_420:      return ("YUV_420");
		case XVIDC_CSF_YONLY:          return ("Y_ONLY");
		case XVIDC_CSF_RGBA:           return ("RGBA");
		case XVIDC_CSF_YCRCBA_444:     return ("YUVA_444");
		case XVIDC_CSF_MEM_RGBX8:      return ("RGBX8");
		case XVIDC_CSF_MEM_YUVX8:      return ("YUVX8");
		case XVIDC_CSF_MEM_YUYV8:      return ("YUYV8");
		case XVIDC_CSF_MEM_RGBA8:      return ("RGBA8");
		case XVIDC_CSF_MEM_YUVA8:      return ("YUVA8");
		case XVIDC_CSF_MEM_RGBX10:     return ("RGBX10");
		case XVIDC_CSF_MEM_YUVX10:     return ("YUVX10");
		case XVIDC_CSF_MEM_RGB565:     return ("RGB565");
		case XVIDC_CSF_MEM_Y_UV8:      return ("Y_UV8");
		case XVIDC_CSF_MEM_Y_UV8_420:  return ("Y_UV8_420");
		case XVIDC_CSF_MEM_RGB8:       return ("RGB8");
		case XVIDC_CSF_MEM_YUV8:       return ("YUV8");
		case XVIDC_CSF_MEM_Y_UV10:     return ("Y_UV10");
		case XVIDC_CSF_MEM_Y_UV10_420: return ("Y_UV10_420");
		case XVIDC_CSF_MEM_Y8:         return ("Y8");
		case XVIDC_CSF_MEM_Y10:        return ("Y10");
		case XVIDC_CSF_MEM_BGRA8:      return ("BGRA8");
		case XVIDC_CSF_MEM_BGRX8:      return ("BGRX8");
		case XVIDC_CSF_MEM_UYVY8:      return ("UYVY8");
		case XVIDC_CSF_MEM_BGR8:       return ("BGR8");
		case XVIDC_CSF_YCBCR_422:      return ("YCBCR_422");
		case XVIDC_CSF_YCBCR_420:      return ("YCBCR_420");
		case XVIDC_CSF_YCBCR_444:      return ("YCBCR_444");
		case XVIDC_CSF_MEM_RGBX12:     return ("RGBX12");
		case XVIDC_CSF_MEM_RGB16:      return ("RGB16");
		case XVIDC_CSF_MEM_YUVX12:     return ("YUVX12");
		case XVIDC_CSF_MEM_YUV16:      return ("YUV16");
		case XVIDC_CSF_MEM_Y_UV12:     return ("Y_UV12");
		case XVIDC_CSF_MEM_Y_UV16:     return ("Y_UV16");
		case XVIDC_CSF_MEM_Y_UV12_420: return ("Y_UV12_420");
		case XVIDC_CSF_MEM_Y_UV16_420: return ("Y_UV16_420");
		case XVIDC_CSF_MEM_Y12:        return ("Y12");
		case XVIDC_CSF_MEM_Y16:        return ("Y16");
		default:
					       return ("Color space format not supported");
	}
}

/******************************************************************************/
