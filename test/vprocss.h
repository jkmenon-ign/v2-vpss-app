
#ifndef XVPROCSS_H /**< prevent circular inclusions by using protection macros*/
#define XVPROCSS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xgpio.h"
#include "xaxis_switch.h"
#include "xvidc.h"
#include "xv_hcresampler_l2.h"
#include "xv_vcresampler_l2.h" 
#include "xv_hscaler_l2.h"
#include "xv_vscaler_l2.h"
#include "xv_csc_l2.h"
#include "vprocss_hw_config.h"

/* HW Reset Network GPIO Channel */
#define GPIO_CH_RESET_SEL            (1u)

/** @name Reset Network
 *
 * @{
 * The following constants define various reset lines in the subsystem
 */
#define XVPROCSS_RSTMASK_VIDEO_IN   (0x01) /**< Reset line going out of vpss */
#define XVPROCSS_RSTMASK_IP_AXIS    (0x02) /**< Reset line for vpss internal video IP blocks */
#define XVPROCSS_RSTMASK_IP_AXIMM   (0x01) /**< Reset line for vpss internal AXI-MM blocks */
/*@}*/

#define XVPROCSS_RSTMASK_ALL_BLOCKS (XVPROCSS_RSTMASK_VIDEO_IN  | \
                                     XVPROCSS_RSTMASK_IP_AXIS)

typedef enum
{
  XVPROCSS_SUBCORE_SCALER_V = 1,
  XVPROCSS_SUBCORE_SCALER_H,
  XVPROCSS_SUBCORE_VDMA,
  XVPROCSS_SUBCORE_LBOX,
  XVPROCSS_SUBCORE_CR_H,
  XVPROCSS_SUBCORE_CR_V_IN,
  XVPROCSS_SUBCORE_CR_V_OUT,
  XVPROCSS_SUBCORE_CSC,
  XVPROCSS_SUBCORE_DEINT,
  XVPROCSS_SUBCORE_MAX
}XVPROCSS_SUBCORE_ID;


/**
 * This typedef enumerates supported scaling modes
 */
typedef enum
{
  XVPROCSS_SCALE_1_1 = 0,
  XVPROCSS_SCALE_UP,
  XVPROCSS_SCALE_DN,
  XVPROCSS_SCALE_NOT_SUPPORTED
}XVprocSs_ScaleMode;


/**
 * This typedef enumerates supported subsystem configuration topology
 */
typedef enum
{
  XVPROCSS_TOPOLOGY_SCALER_ONLY = 0,
  XVPROCSS_TOPOLOGY_FULL_FLEDGED,
  XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY,
  XVPROCSS_TOPOLOGY_CSC_ONLY,
  XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY,
  XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY,
  XVPROCSS_TOPOLOGY_NUM_SUPPORTED
}XVPROCSS_CONFIG_TOPOLOGY;


/**
 * Video Processing Subsystem context scratch pad memory.
 * This contains internal flags, state variables, routing table
 * and other meta-data required by the subsystem. Each instance
 * of the subsystem will have its own context data memory
 */
typedef struct
{
  XVidC_VideoWindow RdWindow; /**< window for Zoom/Pip feature support */
  XVidC_VideoWindow WrWindow; /**< window for Zoom/Pip feature support */

  UINTPTR DeintBufAddr;       /**< Deinterlacer field buffer Addr. in DDR */
  u8 PixelWidthInBits;        /**< Number of bits required to store 1 pixel */

  u8 RtngTable[XVPROCSS_SUBCORE_MAX]; /**< Storage for computed routing map */
  u8 StartCore[XVPROCSS_SUBCORE_MAX]; /**< Enable flag to start sub-core */
  u8 RtrNumCores;             /**< Number of sub-cores in routing map */
  u8 ScaleMode;               /**< Stored computed scaling mode - UP/DN/1:1 */
  u8 ZoomEn;                  /**< Flag to store Zoom feature state */
  u8 PipEn;                   /**< Flag to store PIP feature state */
  u16 VidInWidth;             /**< Input H Active */
  u16 VidInHeight;            /**< Input V Active */
  u16 VidOutWidth;
  u16 VidOutHeight;
  u16 PixelHStepSize;         /**< Increment step size for Pip/Zoom window */
  XVidC_ColorFormat StrmCformat; /**< processing pipe color format */
  XVidC_ColorFormat CscIn;    /**< CSC core input color format */
  XVidC_ColorFormat CscOut;   /**< CSC core output color format */
  XVidC_ColorFormat HcrIn;    /**< horiz. cresmplr core input color format */
  XVidC_ColorFormat HcrOut;   /**< horiz. cresmplr core output color format */
}XVprocSs_ContextData;



/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;   /**< Device ID of the sub-core */
  u32 AddrOffset; /**< sub-core offset from subsystem base address */
}XSubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
  u16 DeviceId;          /**< DeviceId is the unique ID  of the device */
  UINTPTR BaseAddress;   /**< BaseAddress is the physical base address of the
                              subsystem address range */
  UINTPTR HighAddress;   /**< HighAddress is the physical MAX address of the
                              subsystem address range */
  u8 Topology;
  u8 PixPerClock;        /**< Number of Pixels Per Clock processed by Subsystem */
  u16 ColorDepth;        /**< Processing precision of the data pipe */
  u16 NumVidComponents;  /**< Number of Video Components */
  u16 MaxWidth;          /**< Maximum cols supported by subsystem instance */
  u16 MaxHeight;         /**< Maximum rows supported by subsystem instance */
  
  XSubCore Router;
  XSubCore RstAxis;      /**< Axi stream reset network instance configuration */
  XSubCore HCrsmplr;     /**< Sub-core instance configuration */
  XSubCore VCrsmplrIn;   /**< Sub-core instance configuration */
  XSubCore VCrsmplrOut;  /**< Sub-core instance configuration */
  XSubCore Vscale;       /**< Sub-core instance configuration */
  XSubCore Hscale;       /**< Sub-core instance configuration */
  XSubCore Csc;          /**< Sub-core instance configuration */
}XVprocSs_Config;


/* TheXVprocSs driver instance data. The user is required to allocate a variable
 * of this type for everyXVprocSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct
{       
  XVprocSs_Config Config;                /**< Hardware configuration */
  u32 IsReady;                           /**< Device and the driver instance are
                                       initialized */
  XAxis_Switch *RouterPtr;    
  XGpio *RstAxisPtr;                 /**< handle to sub-core driver instance */        
  XV_Hcresampler_l2 *HcrsmplrPtr;    /**< handle to sub-core driver instance */
  XV_Vcresampler_l2 *VcrsmplrInPtr;  /**< handle to sub-core driver instance */
  XV_Vcresampler_l2 *VcrsmplrOutPtr; /**< handle to sub-core driver instance */
  XV_Vscaler_l2 *VscalerPtr;         /**< handle to sub-core driver instance */
  XV_Hscaler_l2 *HscalerPtr;         /**< handle to sub-core driver instance */
  XV_Csc_l2 *CscPtr;
    
  //I/O Streams
  XVidC_VideoStream VidIn;           /**< Input  AXIS configuration */
  XVidC_VideoStream VidOut;          /**< Output AXIS configuration */

  XVprocSs_ContextData CtxtData;

  XVidC_DelayHandler UsrDelayUs;     /**< custom user function for delay/sleep */
  void *UsrTmrPtr;                   /**< handle to timer instance used by user
                                         delay function */    
}XVprocSs;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
 * This macro returns the subsystem topology
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return XVPROCSS_TOPOLOGY_FULL_FLEDGED or XVPROCSS_TOPOLOGY_SCALER_ONLY
 *
 *****************************************************************************/
#define XVprocSs_GetSubsystemTopology(XVprocSsPtr) \
   ((XVprocSsPtr)->Config.Topology)

/*****************************************************************************/
/**
 * This macro returns the subsystem Color Depth
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Color Depth
 *
 *****************************************************************************/
#define XVprocSs_GetColorDepth(XVprocSsPtr) ((XVprocSsPtr)->Config.ColorDepth)

/*****************************************************************************/
                                                                                    
/**
 * This macro checks if subsystem is in Maximum (Full_Fledged) configuration
 *
 * @param  XVprocSsPtr is a pointer to the Video Processing subsystem instance
 *
 * @return Return 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeMax(XVprocSsPtr) \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_FULL_FLEDGED)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in Scaler Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeSscalerOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_SCALER_ONLY)

/*****************************************************************************/
/*  This macro checks if subsystem configuration is in Deinterlace Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeDeinterlaceOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in CSC Only Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeCscOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_CSC_ONLY)

/*****************************************************************************/
/*******************************************************************************/
/**
 * This macro checks if subsystem configuration is in V Chroma ResampleMode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeVCResampleOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY)

/*****************************************************************************/
/**
 * This macro checks if subsystem configuration is in H Chroma Resample Mode
 *
 * @param  XVprocSsPtr is pointer to the Video Processing subsystem instance
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XVprocSs_IsConfigModeHCResampleOnly(XVprocSsPtr)  \
   ((XVprocSsPtr)->Config.Topology == XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY)

/*****************************************************************************/
/*****************************************************************************/
/**
 * This macro sets the specified stream's color format. It can be used to
 * update input or output stream. This call has no side-effect in isolation.
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocSs_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorFormat is the requested color format
 *
 * @return None
 *
 */
/******************************************************************************/
#define XVprocSs_SetStreamColorFormat(Stream, ColorFormat) \
                                        ((Stream)->ColorFormatId = ColorFormat)

/*****************************************************************************/
/**
 * This macro sets the specified stream's color depth. It can be used to update
 * input or output stream. This call has no side-effect in isolation
 * For change to take effect user must trigger processing path reconfiguration
 * by calling XVprocSs_ConfigureSubsystem()
 *
 * @param  Stream is a pointer to the Subsystem Input or Output Stream
 * @param  ColorDepth is the requested color depth
 *
 * @return None
 *
 *****************************************************************************/
#define XVprocSs_SetStreamColorDepth(Stream, ColorDepth) \
                                                    ((Stream)->ColorDepth = ColorDepth) 

int VprocSs_Initialize(XVprocSs *InstancePtr, const char* InstanceName);
int VprocSs_Release(XVprocSs *InstancePtr);

int XVprocSs_CfgInitialize(XVprocSs *InstancePtr,XVprocSs_Config *CfgPtr,
                                   UINTPTR EffectiveAddr);
int XVprocSs_SetSubsystemConfig(XVprocSs *InstancePtr);
XVprocSs_Config* XVprocSs_LookupConfig(u32 DeviceId);

void XVprocSs_Start(XVprocSs *InstancePtr);
void XVprocSs_Stop(XVprocSs *InstancePtr);
void XVprocSs_Reset(XVprocSs *InstancePtr);
void XVprocSs_ReportSubcoreStatus(XVprocSs *InstancePtr,
                                          u32 SubcoreId);
void XVprocSs_ReportSubsystemConfig(XVprocSs *InstancePtr);

int XVprocSs_SetVidStreamIn(XVprocSs *InstancePtr,
                            const XVidC_VideoStream *StrmIn);
int XVprocSs_SetVidStreamOut(XVprocSs *InstancePtr,
                             const XVidC_VideoStream *StrmOut);
int XVprocSs_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId,
                                XVidC_VideoTiming const *Timing);

/* External Filter Load functions */
void XVprocSs_LoadScalerCoeff(XVprocSs *InstancePtr,
                                      u32 CoreId,
                              u16 num_phases,
                              u16 num_taps,
                              const short *Coeff);

void XVprocSs_LoadChromaResamplerCoeff(XVprocSs *InstancePtr,
                                               u32 CoreId,
                                       u16 num_taps,
                                       const short *Coeff);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
                   
