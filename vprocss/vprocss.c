#include "vprocss.h"
#include "vprocss_router.h"
#include "vprocss_coreinit.h"

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
  XAxis_Switch Router;
  XGpio RstAxis;      //Reset for IP's running at AXIS Clk
  
  XV_Hcresampler_l2 Hcrsmplr;
  XV_Vcresampler_l2 VcrsmplrIn;
  XV_Vcresampler_l2 VcrsmplrOut;
  XV_Vscaler_l2 Vscaler;
  XV_Hscaler_l2 Hscaler;
  XV_Csc_l2 Csc;
}XVprocSs_SubCores;

static const char *XVprocSsIpStr[XVPROCSS_SUBCORE_MAX] =  {
    "VidOut",
    "SCALER-V",
    "SCALER-H",
    "VDMA",
    "LBOX",
    "CR-H",
    "CR-VIn",
    "CR-VOut",
    "CSC",
    "DEINT",
  };

#define STEP_PRECISION         (65536)  // 2^16


/**************************** Local Global ***********************************/
//Define Driver instance of all sub-core included in the design */
XVprocSs_SubCores subcoreRepo[XPAR_XVPROCSS_NUM_INSTANCES];


static int SetupModeMax(XVprocSs *XVprocSsPtr);
static void SetPowerOnDefaultState(XVprocSs *XVprocSsPtr);
static void GetIncludedSubcores(XVprocSs *XVprocSsPtr);
static int ValidateSubsystemConfig(XVprocSs *InstancePtr);
static int ValidateScalerOnlyConfig(XVprocSs *XVprocSsPtr);
static int ValidateCscOnlyConfig(XVprocSs *XVprocSsPtr,
                                                                 u16 Allow422,
                                                                 u16 Allow420);
static int ValidateVCResampleOnlyConfig(XVprocSs *XVprocSsPtr);
static int ValidateHCResampleOnlyConfig(XVprocSs *XVprocSsPtr);


static int SetupModeScalerOnly(XVprocSs *XVprocSsPtr);
static int SetupModeCscOnly(XVprocSs *XVprocSsPtr);
static int SetupModeVCResampleOnly(XVprocSs *XVprocSsPtr);
static int SetupModeHCResampleOnly(XVprocSs *XVprocSsPtr);


/*****************************************************************************/
/**
* This macro reads the subsystem reset network state
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
*
* @return Reset state
*           -1: Normal
*           -0: Reset
*
******************************************************************************/
static __inline u32 XVprocSs_GetResetState(XGpio *pReset, u32 channel)
{
  return(XGpio_DiscreteRead(pReset, channel));
}

/*****************************************************************************/
/**
* This macro enables the IP's connected to subsystem reset network
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
* @param  ipBlock is the reset line(s) to be activated
*
* @return None
*
* @note If reset block is not included in the subsystem instance function does
*       not do anything
******************************************************************************/
static __inline void XVprocSs_EnableBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocSs_GetResetState(pReset, channel);
    val |= ipBlock;
    printf("%s: preset val: %x \n",__func__, val);
    XGpio_DiscreteWrite(pReset, channel, val);
  }
}

/*****************************************************************************/
/**
* This macro resets the IP connected to subsystem reset network
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
* @param  ipBlock is the reset line(s) to be asserted
*
* @return None
*
* @note If reset block is not included in the subsystem instance function does
*       not do anything
******************************************************************************/
static __inline void XVprocSs_ResetBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocSs_GetResetState(pReset, channel);
    //printf("XGpio reset reg val %x \n", val);
    val &= ~ipBlock;
    printf("%s: XGpio reset reg val disable %x \n",__func__, val);
    XGpio_DiscreteWrite(pReset, channel, val);

   //read_val = XGpio_DiscreteRead(pReset, channel);
    //printf("XGpio read val : %x \n", read_val);
  }

  
}

/*****************************************************************************/
/**
* This function routes the delay routine used in the subsystem. Preference is
* given to the user registered timer based routine. If no delay handler is
* registered then it uses the platform specific delay handler
*
* @param  XVprocSsPtr is a pointer to the subsystem instance
* @param  msec is delay required
*
* @return None
*
******************************************************************************/
static __inline void WaitUs(XVprocSs *XVprocSsPtr, u32 MicroSeconds)
{
  if(MicroSeconds == 0)
    return;

  if(XVprocSsPtr->UsrDelayUs) {
    /* Use the time handler specified by the user for
     * better accuracy
     */
   XVprocSsPtr->UsrDelayUs(XVprocSsPtr->UsrTmrPtr, MicroSeconds);
  } else {
  /* use default BSP sleep API */
   usleep(MicroSeconds);
  }
}

/************************** Function Definition ******************************/

/******************************************************************************/
void XVprocSs_ReportSubcoreStatus(XVprocSs *InstancePtr,
                                          u32 SubcoreId)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((SubcoreId >= XVPROCSS_SUBCORE_SCALER_V) &&
                         (SubcoreId < XVPROCSS_SUBCORE_MAX));
  
  switch(SubcoreId)
  {
    case XVPROCSS_SUBCORE_SCALER_V:
      XV_VScalerDbgReportStatus(InstancePtr->VscalerPtr);
          break;

    case XVPROCSS_SUBCORE_SCALER_H:
      XV_HScalerDbgReportStatus(InstancePtr->HscalerPtr);
          break;

#if 0
    case XVPROCSS_SUBCORE_VDMA: 
      XVprocSs_VdmaDbgReportStatus(InstancePtr,
                                               InstancePtr->CtxtData.PixelWidthInBits);
          break;

    case XVPROCSS_SUBCORE_LBOX:
      XV_LBoxDbgReportStatus(InstancePtr->LboxPtr);
          break;

    case XVPROCSS_SUBCORE_CR_H:
      XV_HCrsmplDbgReportStatus(InstancePtr->HcrsmplrPtr);
          break;

    case XVPROCSS_SUBCORE_CR_V_IN:
      XV_VCrsmplDbgReportStatus(InstancePtr->VcrsmplrInPtr);
          break;

    case XVPROCSS_SUBCORE_CR_V_OUT:
      XV_VCrsmplDbgReportStatus(InstancePtr->VcrsmplrOutPtr);
          break;

    case XVPROCSS_SUBCORE_CSC:
          XV_CscDbgReportStatus(InstancePtr->CscPtr);
          break;

    case XVPROCSS_SUBCORE_DEINT:
          XV_DeintDbgReportStatus(InstancePtr->DeintPtr);
          break;
#endif
    default:
          break;
  }
}

void XVprocSs_ReportSubsystemConfig(XVprocSs *InstancePtr)
{
  //XVidC_VideoWindow win;
  u32 count;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  printf("\r\n------ SUBSYSTEM INPUT/OUTPUT CONFIG ------\r\n");
  printf("->INPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidIn);
  printf("\r\n->OUTPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidOut);

    count = 0;
    //print IP Data Flow Map
    printf("\r\nData Flow Map: VidIn");
    while(count<InstancePtr->CtxtData.RtrNumCores)
    {
      printf(" -> %s",XVprocSsIpStr[InstancePtr->CtxtData.RtngTable[count++]]);
    }
  printf("\r\n");
}

     
/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void GetIncludedSubcores(XVprocSs *XVprocSsPtr)
{

#if 0
  XVprocSsPtr->HcrsmplrPtr    = ((XVprocSsPtr->Config.HCrsmplr.IsPresent)   \
                               ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].Hcrsmplr)    : NULL);
  XVprocSsPtr->VcrsmplrInPtr  = ((XVprocSsPtr->Config.VCrsmplrIn.IsPresent)  \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].VcrsmplrIn)  : NULL);
  XVprocSsPtr->VcrsmplrOutPtr = ((XVprocSsPtr->Config.VCrsmplrOut.IsPresent) \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].VcrsmplrOut) : NULL);
#endif
  XVprocSsPtr->VscalerPtr     = ((XVprocSsPtr->Config.Vscale.IsPresent)      \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].Vscaler)     : NULL);
#if 0 
XVprocSsPtr->CscPtr         = ((XVprocSsPtr->Config.Csc.IsPresent)         \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].Csc)         : NULL); 
#endif
 XVprocSsPtr->HscalerPtr     = ((XVprocSsPtr->Config.Hscale.IsPresent)      \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].Hscaler)     : NULL);
#if 0
 XVprocSsPtr->RouterPtr      = ((XVprocSsPtr->Config.Router.IsPresent)      \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].Router)      : NULL);
#endif
 XVprocSsPtr->RstAxisPtr     = ((XVprocSsPtr->Config.RstAxis.IsPresent)     \
                              ? (&subcoreRepo[XVprocSsPtr->Config.DeviceId].RstAxis)     : NULL);
}




int XVprocSs_CfgInitialize(XVprocSs *InstancePtr,XVprocSs_Config *CfgPtr,
                                   UINTPTR EffectiveAddr)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

  const char topo_name[XVPROCSS_TOPOLOGY_NUM_SUPPORTED][32]={
                "Scaler-only",
                "Full Fledged",
                "Deinterlacer-only",
                "Csc-only",
                "VCResample-only",
                "HCResample-only"};


  // /* Log the start of initialization */
  ////XVprocSs_LogWrite(InstancePtr,XVprocSs_EVT_INIT,XVprocSs_EDAT_BEGIN);
  printf("Info: Subsystem start init \n");


  /* Setup the instance */
  InstancePtr->Config = *CfgPtr;
  InstancePtr->Config.BaseAddress = EffectiveAddr;

  InstancePtr->Config.DeviceId = 0;

  if(XVprocSs_GetSubsystemTopology(InstancePtr) >= XVPROCSS_TOPOLOGY_NUM_SUPPORTED) {
      printf("failed to get topology \n");;
      return(XST_FAILURE);
  }
  printf("Topology is %s \n", topo_name[XVprocSs_GetSubsystemTopology(InstancePtr)]);


  /* Determine sub-cores included in the provided instance of subsystem */
  GetIncludedSubcores(InstancePtr);

  printf("locating subcores \n");
  /* Initialize all included sub_cores */
  if(InstancePtr->RstAxisPtr) {
	printf("reset axis subcore init \n");
        if(XVprocSs_SubcoreInitResetAxis(InstancePtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if(InstancePtr->HscalerPtr) {
	printf("H-scaler subcore-init \n");
        if(XVprocSs_SubcoreInitHScaler(InstancePtr) != XST_SUCCESS) {
       return(XST_FAILURE);
     }
   }

  if(InstancePtr->VscalerPtr) {
	printf("V-scaler subcore init \n");
        if(XVprocSs_SubcoreInitVScaler(InstancePtr) != XST_SUCCESS) {
          return(XST_FAILURE);
         }
   }

#if 0
  if(InstancePtr->RouterPtr) {
        if(XVprocSs_SubcoreInitRouter(InstancePtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  
  if(InstancePtr->CscPtr) {
	 
	printf("CSC subcore-init \n");
        if(XVprocSs_SubcoreInitCsc(InstancePtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if(InstancePtr->HcrsmplrPtr) {
        printf("hcrsampler coreinit \n");
        if(XVprocSs_SubcoreInitHCrsmplr(InstancePtr) != XST_SUCCESS) {
          return(XST_FAILURE);
        }
  }
  if(InstancePtr->VcrsmplrInPtr) {
        if(XVprocSs_SubcoreInitVCrsmpleIn(InstancePtr) != XST_SUCCESS) {
          return(XST_FAILURE);
        }
  }

  if(InstancePtr->VcrsmplrOutPtr) {
        if(XVprocSs_SubcoreInitVCrsmpleOut(InstancePtr) != XST_SUCCESS) {
          return(XST_FAILURE);
        }
  }
 #endif
/* Reset the hardware */
   XVprocSs_Reset(InstancePtr);

  /* Set subsystem to power on default state */
  SetPowerOnDefaultState(InstancePtr);

  /* Set the flag to indicate the subsystem is ready */
  InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

  printf("Info: Subsystem ready\n");
  return(XST_SUCCESS);

}



/*****************************************************************************/
/**
* This function configures the Video Processing subsystem internal blocks
* to power on default configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void SetPowerOnDefaultState(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoStream vidStrmIn;
  XVidC_VideoStream vidStrmOut;
  //XVidC_VideoWindow win;
  //u16 PixPrecisionIndex;
  XVidC_VideoTiming const *TimingPtr;

  printf("%s \n", __func__);

  memset(&vidStrmIn,  0, sizeof(XVidC_VideoStream));
  memset(&vidStrmOut, 0, sizeof(XVidC_VideoStream));

  /* Setup Default Output Stream configuration */
  vidStrmOut.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmOut.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmOut.FrameRate     = XVIDC_FR_60HZ;
  vidStrmOut.IsInterlaced  = FALSE;
  vidStrmOut.ColorDepth    = (XVidC_ColorDepth)XVprocSsPtr->Config.ColorDepth;
  vidStrmOut.PixPerClk     = (XVidC_PixelsPerClock)XVprocSsPtr->Config.PixPerClock;

  TimingPtr = XVidC_GetTimingInfo(vidStrmOut.VmId);
  vidStrmOut.Timing = *TimingPtr;

  /* Setup Default Input Stream configuration */
  vidStrmIn.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmIn.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmIn.FrameRate     = XVIDC_FR_60HZ;
  vidStrmIn.IsInterlaced  = FALSE;
  vidStrmIn.ColorDepth    = (XVidC_ColorDepth)XVprocSsPtr->Config.ColorDepth;
  vidStrmIn.PixPerClk     = (XVidC_PixelsPerClock)XVprocSsPtr->Config.PixPerClock;

  TimingPtr = XVidC_GetTimingInfo(vidStrmIn.VmId);
  vidStrmIn.Timing = *TimingPtr;

  /* Setup Video Processing subsystem input/output  configuration */
  XVprocSs_SetVidStreamIn(XVprocSsPtr,  &vidStrmIn);
  XVprocSs_SetVidStreamOut(XVprocSsPtr, &vidStrmOut);

  /* compute data width supported by Vdma */
  XVprocSsPtr->CtxtData.PixelWidthInBits = XVprocSsPtr->Config.NumVidComponents *
                                                       XVprocSsPtr->VidIn.ColorDepth;
  switch(XVprocSsPtr->Config.PixPerClock)
  {
    case XVIDC_PPC_1:
    case XVIDC_PPC_2:
         if(XVprocSsPtr->Config.ColorDepth == XVIDC_BPC_10)
         {
            /*Align the bit width to next byte boundary for this particular case
            * Num_Channel       Color Depth             PixelWidth              Align
            * ----------------------------------------------------
            *    2                              10                              20                       24
            *    3                              10                              30                       32
            *
            *    HW will do the bit padding for 20->24 and 30->32
            */
           XVprocSsPtr->CtxtData.PixelWidthInBits = ((XVprocSsPtr->CtxtData.PixelWidthInBits + 7)/8)*8;
         }
         break;

    default:
         break;
  }


  /* Release reset before programming any IP Block */
  XVprocSs_EnableBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_ALL_BLOCKS);
  
  printf("setting power on default \n");
}                                                                                                                                                                                         


/****************************************************************************/
/**
* This function starts the video subsystem including all sub-cores that are
* included in the processing pipeline for a given use-case. Video pipe is
* started from back to front
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
* @note Cores are started only if the corresponding start flag in the scratch
*       pad memory is set. This allows to selectively start only those cores
*       included in the processing chain
******************************************************************************/
void XVprocSs_Start(XVprocSs *InstancePtr)
{
  u8 *StartCorePtr;

  printf("starting subcores \n");  
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  StartCorePtr = &InstancePtr->CtxtData.StartCore[0];

 if(StartCorePtr[XVPROCSS_SUBCORE_CR_V_OUT]){
    printf("starting V-resampler out \n");
    XV_VCrsmplStart(InstancePtr->VcrsmplrOutPtr);
 }
 
 if(StartCorePtr[XVPROCSS_SUBCORE_CR_H]){
	printf("starting h-resampler \n"); 
   	XV_HCrsmplStart(InstancePtr->HcrsmplrPtr);
 }

 if(StartCorePtr[XVPROCSS_SUBCORE_CSC]){
     printf("starting csc \n");
     XV_CscStart(InstancePtr->CscPtr);
 }

 if(StartCorePtr[XVPROCSS_SUBCORE_SCALER_H]){
         printf("starting h-scaler \n");
 	 XV_HScalerStart(InstancePtr->HscalerPtr);
 }

 if(StartCorePtr[XVPROCSS_SUBCORE_SCALER_V]){
 	printf("starting v-scaler \n"); 
        XV_VScalerStart(InstancePtr->VscalerPtr);
 }

 if(StartCorePtr[XVPROCSS_SUBCORE_CR_V_IN]){
       printf("starting v-resample -In \n");
       XV_VCrsmplStart(InstancePtr->VcrsmplrInPtr);
 }

 /* Subsystem ready to accept axis - Enable Video Input */
  XVprocSs_EnableBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_VIDEO_IN);

}

/*****************************************************************************/
/**
* This function stops the video subsystem including all sub-cores
* Stop the video pipe starting from front to back
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_Stop(XVprocSs *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(InstancePtr->VcrsmplrInPtr)
    XV_VCrsmplStop(InstancePtr->VcrsmplrInPtr);
  
  if(InstancePtr->VscalerPtr)
    XV_VScalerStop(InstancePtr->VscalerPtr);

  if(InstancePtr->HscalerPtr)
    XV_HScalerStop(InstancePtr->HscalerPtr);

  if(InstancePtr->HcrsmplrPtr)
    XV_HCrsmplStop(InstancePtr->HcrsmplrPtr);
  
  if(InstancePtr->VcrsmplrOutPtr)
    XV_VCrsmplStop(InstancePtr->VcrsmplrOutPtr);

}

/*****************************************************************************/
/**
* This function resets the video subsystem sub-cores. There are 2 reset
* networks within the subsystem
*  - For cores that are on AXIS interface
*  - For cores that are on AXI-MM interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
*   XVprocSs_Reset,_Start controls vpss resets as shown
*    axis_int ----_______________________--------------------------------
*    axis_ext ----________________________________________________-------
*    aximm    -----------_______-----------------------------------------
*                 | 100us| 100us| 1000us | 1000us |               |
*            _Reset...............................|               |
*                                                  Program VSPP IP|
*                                                            _Start
*
******************************************************************************/
void XVprocSs_Reset(XVprocSs *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


  printf("%s \n", __func__);

  /* Reset All IP Blocks on AXIS interface and wait before doing the aximm reset*/
  XVprocSs_ResetBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_ALL_BLOCKS);
  WaitUs(InstancePtr, 100); /* hold reset line for 100us before resetting Aximm */

   //WaitUs(InstancePtr, 100); /* hold reset line for 100us */
  /*
   * Make sure the video IP's are out of reset - IP's cannot be programmed when held
   * in reset. Will cause Axi-Lite bus to lock.
   * Release IP reset - but hold vid_in in reset
   */
  XVprocSs_EnableBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_IP_AXIS);
  WaitUs(InstancePtr, 1000); /* wait 1ms for AXIS to stabilize */

  /* Reset start core flags */
  memset(InstancePtr->CtxtData.StartCore, 0, sizeof(InstancePtr->CtxtData.StartCore));

  printf("XGpio Reset done \n");
  

   printf("Info: Subsystem reset \n");
  //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_RESET_VPSS, XVPROCSS_EDAT_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the video subsystem input interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmIn is the pointer to input stream configuration
*
* @return XST_SUCCESS
*
******************************************************************************/
int XVprocSs_SetVidStreamIn(XVprocSs *InstancePtr,
                            const XVidC_VideoStream *StrmIn)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmIn != NULL);
  Xil_AssertNonvoid((StrmIn->Timing.HActive > 0) &&
                    (StrmIn->Timing.VActive > 0));

  /* set stream properties */
  InstancePtr->VidIn = *StrmIn;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the video subsystem output interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmOut is the pointer to input stream configuration
*
* @return XST_SUCCESS
*
******************************************************************************/
int XVprocSs_SetVidStreamOut(XVprocSs *InstancePtr,
                             const XVidC_VideoStream *StrmOut)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmOut != NULL);
  Xil_AssertNonvoid((StrmOut->Timing.HActive > 0) &&
                    (StrmOut->Timing.VActive > 0));

  /* set stream properties */
  InstancePtr->VidOut = *StrmOut;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function sets the required subsystem video stream to the user provided
* stream and timing parameters
*
* @param  StreamPtr is a pointer to the subsystem video stream to be configured
* @param  VmId is the Video Mode ID of the new resolution to be set
* @param  Timing is the timing parameters of the new resolution to be set

* @return XST_SUCCESS if successful else XST_FAILURE
******************************************************************************/
int XVprocSs_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId,
                                 XVidC_VideoTiming const *Timing)
{
  /* Verify arguments */
  Xil_AssertNonvoid(StreamPtr != NULL);
  Xil_AssertNonvoid(Timing != NULL);
  Xil_AssertNonvoid((Timing->HActive > 0) &&
                    (Timing->VActive > 0));

  /* Video Mode could be from the list of supported modes or custom */
  if(VmId != XVIDC_VM_NOT_SUPPORTED) {
      /* update stream timing properties */
      StreamPtr->VmId   = VmId;
      StreamPtr->Timing = *Timing;
      return(XST_SUCCESS);
  } else {
      return(XST_FAILURE);
  }
}


/*****************************************************************************/
/**
* This function configures the video subsystem pipeline for Maximum
* (Full_Fledged) topology
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
static int SetupModeMax(XVprocSs *XVprocSsPtr)
{
  int status;

  printf("%s \n", __func__);
  /* Build Routing table for the Video Data Flow */
  status = XVprocSs_BuildRoutingTable(XVprocSsPtr);

  if(status == XST_SUCCESS) {
    /* Reset the IP Blocks inside the VPSS */
    XVprocSs_Reset(XVprocSsPtr);

    /* Set the Video Data Router registers */
    XVprocSs_ProgRouterMux(XVprocSsPtr);

    /* program the Video IP subcores according to the use case */
    XVprocSs_SetupRouterDataFlow(XVprocSsPtr);
  }
 
  printf("VPSS configure for MAX topology \n");
  return(status);
}


/*****************************************************************************/
/**
* This function is the entry point into the video processing subsystem driver
* processing path. It will examine the instantiated subsystem configuration mode
* and the input and output stream configuration. Based on the available
* information control flow is determined and requisite sub-cores are configured
* to implement the supported use case
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
#if 0
int XVprocSs_SetSubsystemConfig(XVprocSs *InstancePtr)
{
  int status = XST_SUCCESS;
  
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
 
#if 0 
  /* validate subsystem configuration */
  if(ValidateSubsystemConfig(InstancePtr) != XST_SUCCESS) {
        return(XST_FAILURE);
  }
 
#endif

  status = SetupModeMax(InstancePtr);

  return(status);
}
#endif

static int ValidateSubsystemConfig(XVprocSs *InstancePtr)
{

  printf("%s \n", __func__);
  XVidC_VideoStream *StrmIn  = &InstancePtr->VidIn;
  XVidC_VideoStream *StrmOut = &InstancePtr->VidOut;

  /* Runtime Color Depth conversion not supported
   * Always overwrite input/output stream color depth with subsystem setting
   */
  StrmIn->ColorDepth  = (XVidC_ColorDepth)InstancePtr->Config.ColorDepth;
  StrmOut->ColorDepth = (XVidC_ColorDepth)InstancePtr->Config.ColorDepth;

  /* Runtime Pixel/Clock conversion not supported
   * Always overwrite input/output stream pixel/clk with subsystem setting
   */
  StrmIn->PixPerClk  = (XVidC_PixelsPerClock)InstancePtr->Config.PixPerClock;
  StrmOut->PixPerClk = (XVidC_PixelsPerClock)InstancePtr->Config.PixPerClock;

  /* Frame rate conversion is possible only in FULL topology */
  if((XVprocSs_GetSubsystemTopology(InstancePtr)
      != XVPROCSS_TOPOLOGY_FULL_FLEDGED) &&
     (StrmIn->FrameRate != StrmOut->FrameRate)) {
      printf("Error: Not Full mode, and Input & Output Frame Rate different \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_FRDIFF);
    return(XST_FAILURE);
  }
 
 /* Check input resolution is supported by HW */
  if((StrmIn->Timing.HActive > InstancePtr->Config.MaxWidth) ||
         (StrmIn->Timing.HActive == 0) ||
         (StrmIn->Timing.VActive > InstancePtr->Config.MaxHeight) ||
         (StrmIn->Timing.VActive == 0)) {
     printf("Error: Input Stream Resolution out of range 0...MAX \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_IVRANGE);
        return(XST_FAILURE);
  }

  /* Check output resolution is supported by HW */
  if((StrmOut->Timing.HActive > InstancePtr->Config.MaxWidth) ||
         (StrmOut->Timing.HActive == 0) ||
         (StrmOut->Timing.VActive > InstancePtr->Config.MaxHeight) ||
         (StrmOut->Timing.VActive == 0)) {
        printf("Error: Output Stream Resolution out of range 0...MAX \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_OVRANGE);
        return(XST_FAILURE);
  }

  /* Check Stream Width is aligned at Samples/Clock boundary */
  if(((StrmIn->Timing.HActive  % InstancePtr->Config.PixPerClock) != 0) ||
     ((StrmOut->Timing.HActive % InstancePtr->Config.PixPerClock) != 0))
  {
        printf("Error: Input/Output Width not aligned with Samples/Clk  \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_WIDBAD);
        return(XST_FAILURE);
  }

  /* Check for HCResamp required, but not present */
  if(XVprocSs_IsConfigModeMax(InstancePtr) &&
     ((StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) || (StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_422)) &&
     ((StrmOut->ColorFormatId == XVIDC_CSF_YCRCB_444) || (StrmOut->ColorFormatId == XVIDC_CSF_RGB)) &&
     (InstancePtr->HcrsmplrPtr==NULL)) {
      printf("Error: HCResampler required but not found \n");
     //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_NOHCR);
        return(XST_FAILURE);
  }

  /* Check for YUV422 In/Out stream width is even */
  if(((StrmIn->ColorFormatId  == XVIDC_CSF_YCRCB_422) &&
          ((StrmIn->Timing.HActive % 2) != 0)) ||
          ((StrmOut->ColorFormatId == XVIDC_CSF_YCRCB_422) &&
      ((StrmOut->Timing.HActive % 2) != 0))) {
        printf("Error: YUV422 stream width must be even \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_WIDODD);
        return(XST_FAILURE);
  }

  /* Check for YUV420 In stream width and height is even */
  if((StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) &&
      (((StrmIn->Timing.HActive % 2) != 0) &&
      ((StrmIn->Timing.VActive % 2) != 0))) {
      printf("Error: YUV420 input width and height must be even \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_SIZODD);
    return(XST_FAILURE);
  }
  
   /* Check for VCResamp required, but not present */
  /* In the Full-fledged case, the Output V C Resampler is "VcrsmplrOut" */
  /* In the VCResample-only case, the Output V C Resampler is "VcrsmplrIn" */
  if(((StrmOut->ColorFormatId == XVIDC_CSF_YCRCB_420) &&
      XVprocSs_IsConfigModeMax(InstancePtr) &&
      !InstancePtr->VcrsmplrOutPtr)) {
      printf("Error: VCResampler required at output but not found \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_NOVCRO);
        return(XST_FAILURE);
  }
  if(((StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) &&
      XVprocSs_IsConfigModeMax(InstancePtr) &&
      !InstancePtr->VcrsmplrInPtr)) {
        printf("Error: VCResampler required at input but not found \n");
    //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VPSS_NOVCRI);
        return(XST_FAILURE);
  }

  /* Check for Interlaced input limitation */
  if(StrmIn->IsInterlaced) {
        if(StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) {
          printf("Error: Interlaced YUV420 stream not supported \n");
	   //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_NO420);
          return(XST_FAILURE);
        }
  }
  printf("Info: Subsystem configuration is valid \n");
 // XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VPSS, XVPROCSS_EDAT_VALID);
  return(XST_SUCCESS);
}
                                                            
#if 1

int XVprocSs_SetSubsystemConfig(XVprocSs *InstancePtr)
{
  int status = XST_SUCCESS;

  printf("%s \n", __func__);
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* validate subsystem configuration */
  if(ValidateSubsystemConfig(InstancePtr) != XST_SUCCESS) {
        return(XST_FAILURE);
  }

  switch(XVprocSs_GetSubsystemTopology(InstancePtr))
  {
    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
        status = SetupModeMax(InstancePtr);
        break;

    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
        //Only configuration supported is Scaler-only
        status = SetupModeScalerOnly(InstancePtr);
        break;

    case XVPROCSS_TOPOLOGY_CSC_ONLY:
        //Only configuration supported is CSC-only
        status = SetupModeCscOnly(InstancePtr);
        break;


    case XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY:
        //Only configurations supported are 420 <-> 422
        status = SetupModeVCResampleOnly(InstancePtr);
        break;

    case XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY:
        //Only configurations supported are 422 <-> 444
        status = SetupModeHCResampleOnly(InstancePtr);
        break;

    default:
      //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CHK_TOPO, XVPROCSS_EDAT_FAILURE);
      status = XST_FAILURE;
      break;
  }
  return(status);
}

#endif

/*****************************************************************************/
/**
* This function enables user to load external filter coefficients for
* Scaler cores, independently for H & V
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CoreId is the Scaler core to be worked on
*           - XVPROCSS_SUBCORE_SCALER_V
*           - XVPROCSS_SUBCORE_SCALER_H
* @param  num_phases is the number of phases supported by Scaler
* @param  num_taps is the effective taps to be used by Scaler
* @param  Coeff is the pointer to the filter table to be loaded
*
* @return None
*
* @note   Applicable only if Scaler cores are included in the subsystem
*
******************************************************************************/
void XVprocSs_LoadScalerCoeff(XVprocSs *InstancePtr,
                          u32 CoreId,
                              u16 num_phases,
                              u16 num_taps,
                              const short *Coeff)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(Coeff != NULL);
  Xil_AssertVoid((CoreId == XVPROCSS_SUBCORE_SCALER_V) ||
             (CoreId == XVPROCSS_SUBCORE_SCALER_H));

  switch(CoreId)
  {
    case XVPROCSS_SUBCORE_SCALER_V:
      if(InstancePtr->VscalerPtr) {
      XV_VScalerLoadExtCoeff(InstancePtr->VscalerPtr,
                           num_phases,
                           num_taps,
                           Coeff);
        //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_LDCOEF);
      } else {
        //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_IPABSENT);
      }
    break;

    case XVPROCSS_SUBCORE_SCALER_H:
    if(InstancePtr->HscalerPtr) {
      XV_HScalerLoadExtCoeff(InstancePtr->HscalerPtr,
                           num_phases,
                           num_taps,
                           Coeff);
        //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_LDCOEF);
      } else {
        //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_IPABSENT);
    }
    break;

    default:
    break;
  }
}

/*****************************************************************************/
/**
* This function enables user to load external filter coefficients for
* Chroma Resampler cores, independently for H & V
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CoreId is the resampler core to be worked on
*           - XVPROCSS_SUBCORE_CR_H
*           - XVPROCSS_SUBCORE_CR_V_IN
*           - XVPROCSS_SUBCORE_CR_V_OUT
* @param  num_taps is the taps of the resampler hw instance
* @param  Coeff is the pointer to the filter table to be loaded
*
* @return None
*
* @note   Applicable only if Resampler cores are included in the subsystem
*
******************************************************************************/
void XVprocSs_LoadChromaResamplerCoeff(XVprocSs *InstancePtr,
                                   u32 CoreId,
                                       u16 num_taps,
                                       const short *Coeff)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(Coeff != NULL);
  Xil_AssertVoid((CoreId == XVPROCSS_SUBCORE_CR_H)    ||
           (CoreId == XVPROCSS_SUBCORE_CR_V_IN) ||
           (CoreId == XVPROCSS_SUBCORE_CR_V_OUT));

  switch(CoreId)
  {
  case XVPROCSS_SUBCORE_CR_H:
    if(InstancePtr->HcrsmplrPtr) {
      XV_HCrsmplrLoadExtCoeff(InstancePtr->HcrsmplrPtr, num_taps, Coeff);
          //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_LDCOEF);
    } else {
          //XVprocSs_LogWrite(InstancePtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_IPABSENT);
    }
    break;


  default:
      break;
  }
}



static int SetupModeScalerOnly(XVprocSs *XVprocSsPtr)
{
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut, hsc_ColorFormatIn;
  int status = XST_SUCCESS;
  u32 pixel_rate = 0;
  u32 line_rate;

  printf("%s \n", __func__);
  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  if(!XVprocSsPtr->VscalerPtr) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_IPABSENT);
    return(XST_FAILURE);
  }

  if(!XVprocSsPtr->HscalerPtr) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_IPABSENT);
    return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = ValidateScalerOnlyConfig(XVprocSsPtr);

  //XVprocSs_ResetBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_ALL_BLOCKS);
  
  if(status ==  XST_SUCCESS)
  {
    /* Reset the IP Blocks inside the VPSS */
    XVprocSs_Reset(XVprocSsPtr);

    /* UpScale mode V Scaler is before H Scaler */
    vsc_WidthIn   = XVprocSsPtr->VidIn.Timing.HActive;
    vsc_HeightIn  = XVprocSsPtr->VidIn.Timing.VActive;
    vsc_HeightOut = XVprocSsPtr->VidOut.Timing.VActive;

    hsc_WidthIn  = vsc_WidthIn;
    hsc_HeightIn = vsc_HeightOut;
    hsc_WidthOut = XVprocSsPtr->VidOut.Timing.HActive;
    if (XVprocSsPtr->VidIn.ColorFormatId == XVIDC_CSF_YCRCB_420)
    {
        hsc_ColorFormatIn = XVIDC_CSF_YCRCB_422;
    }
    else
    {
        hsc_ColorFormatIn = XVprocSsPtr->VidIn.ColorFormatId;
    }

printf("configuring vscaler to widthin: %d, heightin: %d heightout: %d, format id: %d \n", vsc_WidthIn,vsc_HeightIn,vsc_HeightOut,XVprocSsPtr->VidIn.ColorFormatId);
    /* Configure scaler to scale input to output resolution */

#if 1
    XV_VScalerSetup(XVprocSsPtr->VscalerPtr,
                    vsc_WidthIn,
                    vsc_HeightIn,
                    vsc_HeightOut,
                    XVprocSsPtr->VidIn.ColorFormatId);
#endif
printf("configuring hscaler to heightin: %d, widthin: %d widthout: %d, formatin id: %d, formatout id: %d \n", hsc_HeightIn,hsc_WidthIn,hsc_WidthOut,hsc_ColorFormatIn,XVprocSsPtr->VidOut.ColorFormatId);
#if 1
    XV_HScalerSetup(XVprocSsPtr->HscalerPtr,
                    hsc_HeightIn,
                    hsc_WidthIn,
                    hsc_WidthOut,
                    hsc_ColorFormatIn,
                    XVprocSsPtr->VidOut.ColorFormatId);
#endif	

 
#if 0 
    XV_vscaler_WriteReg(XVprocSsPtr->VscalerPtr->Vsc.Config.BaseAddress,
				XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA, vsc_HeightIn);
    XV_vscaler_WriteReg(XVprocSsPtr->VscalerPtr->Vsc.Config.BaseAddress,
				XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA, vsc_WidthIn);
   
     XV_vscaler_WriteReg(XVprocSsPtr->VscalerPtr->Vsc.Config.BaseAddress,
				XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA, vsc_HeightOut);
     
    line_rate = (vsc_HeightIn * STEP_PRECISION) / vsc_HeightOut;
   XV_vscaler_WriteReg(XVprocSsPtr->VscalerPtr->Vsc.Config.BaseAddress,
				XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA,line_rate);
    XV_vscaler_WriteReg(XVprocSsPtr->VscalerPtr->Vsc.Config.BaseAddress,
				XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA,XVprocSsPtr->VidIn.ColorFormatId);


    pixel_rate = (vsc_WidthIn * STEP_PRECISION) / hsc_WidthOut; 
#if 0
    CalculatePhases(XVprocSsPtr->HscalerPtr,hsc_WidthIn,hsc_WidthOut, pixel_rate);
 
    XV_HScalerSetPhase(XVprocSsPtr->HscalerPtr);
#endif		
  

  XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA, hsc_HeightIn);  
   
    XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				 XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA, hsc_WidthIn);  
   XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				 XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA, hsc_WidthOut);  



  

    XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				 XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA, pixel_rate);  


 #if 1
    XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				 XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA,XVprocSsPtr->VidIn.ColorFormatId);  
#endif

    XV_hscaler_WriteReg(XVprocSsPtr->HscalerPtr->Hsc.Config.BaseAddress,
				 XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA,XVprocSsPtr->VidOut.ColorFormatId);  

#if 0
    pixel_rate = (vsc_WidthIn * STEP_PRECISION) / hsc_WidthOut; 

    CalculatePhases(XVprocSsPtr->HscalerPtr,hsc_WidthIn,hsc_WidthOut, pixel_rate);
 
    XV_HScalerSetPhase(XVprocSsPtr->HscalerPtr);
#endif

#endif
   /* Start Scaler sub-cores */
    XV_HScalerStart(XVprocSsPtr->HscalerPtr);
    XV_VScalerStart(XVprocSsPtr->VscalerPtr);

    /* Subsystem Ready to accept input stream - Enable Video Input */
    //XVprocSs_EnableBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_VIDEO_IN);
    printf("Info: Scalers start \n"); 

    XVprocSs_EnableBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, XVPROCSS_RSTMASK_VIDEO_IN);
   //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_SETUPOK);
  }
  return(status);
}

static int SetupModeHCResampleOnly(XVprocSs *XVprocSsPtr)
{
  XVidC_ColorFormat HcrIn, HcrOut;
  u32 HeightOut = 0;
  u32 WidthOut = 0;
  int status = XST_SUCCESS;

  if(!XVprocSsPtr->HcrsmplrPtr) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_IPABSENT);
    return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = ValidateHCResampleOnlyConfig(XVprocSsPtr);

  if(status ==  XST_SUCCESS) {
    /* In the single-IP cases the reset has been done outside this routine */

    HcrIn = XVprocSsPtr->VidIn.ColorFormatId;
    HcrOut = XVprocSsPtr->VidOut.ColorFormatId;
    HeightOut = XVprocSsPtr->VidOut.Timing.VActive;
    WidthOut = XVprocSsPtr->VidOut.Timing.HActive;

    /* Configure H chroma resampler in and out color space */
    XV_HCrsmplSetFormat(XVprocSsPtr->HcrsmplrPtr,
                        HcrIn,
                        HcrOut);

    XV_HCrsmplSetActiveSize(XVprocSsPtr->HcrsmplrPtr,
                            WidthOut,
                            HeightOut);
       /* Start chroma resampler sub-core */
    XV_HCrsmplStart(XVprocSsPtr->HcrsmplrPtr);
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_SETUPOK);
  } 

  return(status);
}

static int SetupModeVCResampleOnly(XVprocSs *XVprocSsPtr)
{
  XVprocSs_ContextData *CtxtPtr = &XVprocSsPtr->CtxtData;
  int status = XST_SUCCESS;

  if(!XVprocSsPtr->VcrsmplrInPtr) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_IPABSENT);
    return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = ValidateVCResampleOnlyConfig(XVprocSsPtr);

  if(status ==  XST_SUCCESS) {
    /* In the single-IP cases the reset has been done outside this routine */

        CtxtPtr->VidInWidth  = XVprocSsPtr->VidIn.Timing.HActive;
        CtxtPtr->VidInHeight = XVprocSsPtr->VidIn.Timing.VActive;

    /* Configure V chroma resampler in and out color space */
    XV_VCrsmplSetActiveSize(XVprocSsPtr->VcrsmplrInPtr,
                             CtxtPtr->VidInWidth,
                                CtxtPtr->VidInHeight);

    XV_VCrsmplSetFormat(XVprocSsPtr->VcrsmplrInPtr,
                        XVprocSsPtr->VidIn.ColorFormatId,
                        XVprocSsPtr->VidOut.ColorFormatId);

    /* Start V Chroma Resampler-In sub-core */
    XV_VCrsmplStart(XVprocSsPtr->VcrsmplrInPtr);
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_SETUPOK);
  } 


  return(status);
}


                                                                                       
static int SetupModeCscOnly(XVprocSs *XVprocSsPtr)
{
  XVidC_ColorFormat CscIn, CscOut;
  XVidC_ColorStd StdIn, StdOut;
  XVidC_ColorRange RangeOut;
  XVidC_ColorDepth ColorDepth;
  u32 HeightOut = 0;
  u32 WidthOut = 0;
  u16 Allow422;
  u16 Allow420;
  int status = XST_SUCCESS;

  if(!XVprocSsPtr->CscPtr) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_IPABSENT);
    return(XST_FAILURE);
  }

  Allow422 = XV_CscIs422Enabled(XVprocSsPtr->CscPtr);
  Allow420 = XV_CscIs420Enabled(XVprocSsPtr->CscPtr);

  /* check if input/output stream configuration is supported */
  status = ValidateCscOnlyConfig(XVprocSsPtr, Allow422, Allow420);

  if(status ==  XST_SUCCESS) {
    /* In the single-IP cases the reset has been done outside this routine */

        // when setting up a new resolution, start with default picture settings
    XV_CscSetPowerOnDefaultState(XVprocSsPtr->CscPtr);

        // set the proper color depth: get it from the vprocss config
        ColorDepth = (XVidC_ColorDepth)XVprocSs_GetColorDepth(XVprocSsPtr);
      XV_CscSetColorDepth(XVprocSsPtr->CscPtr, ColorDepth);

        // all other picture settings are filled in by XV_CscSetColorspace
    CscIn = XVprocSsPtr->VidIn.ColorFormatId;
    CscOut = XVprocSsPtr->VidOut.ColorFormatId;
    StdIn = XVprocSsPtr->CscPtr->StandardIn;
    StdOut = XVprocSsPtr->CscPtr->StandardOut;
    RangeOut = XVprocSsPtr->CscPtr->OutputRange;
    XV_CscSetColorspace(XVprocSsPtr->CscPtr,
                        CscIn, CscOut,
                        StdIn, StdOut,
                        RangeOut);

        // set the Global Window size
        HeightOut = XVprocSsPtr->VidOut.Timing.VActive;
    WidthOut = XVprocSsPtr->VidOut.Timing.HActive;
    XV_CscSetActiveSize(XVprocSsPtr->CscPtr,
                        WidthOut,
                        HeightOut);

    /* Start Csc sub-core */
    XV_CscStart(XVprocSsPtr->CscPtr);
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_SETUPOK);
  } 
  return(status);
}


static int ValidateScalerOnlyConfig(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoStream *pStrmIn  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *pStrmOut = &XVprocSsPtr->VidOut;

  if ((pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) && !XV_VscalerIs420Enabled(XVprocSsPtr->VscalerPtr)) {
    printf("INFO: Vscaler config no 420 \n");
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_NO420);
    return(XST_FAILURE);
  }

  if ((pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_422) && !XV_HscalerIs422Enabled(XVprocSsPtr->HscalerPtr)) {
    printf("Scaler error: 422 color format not allowed \n");
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_NO422);
    return(XST_FAILURE);
  }

  if (!XV_HScalerValidateConfig(XVprocSsPtr->HscalerPtr, pStrmIn->ColorFormatId, pStrmOut->ColorFormatId))
  {
    printf("Error: HScaler init failed \n");
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_FAILURE);
    return(XST_FAILURE);
  }

   printf("Info: Scaler-only configuration is valid \n");

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_VALID);
  return(XST_SUCCESS);
}


static int ValidateCscOnlyConfig(XVprocSs *XVprocSsPtr,
                                 u16 Allow422,
                                 u16 Allow420)
{
  XVidC_VideoStream *pStrmIn  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *pStrmOut = &XVprocSsPtr->VidOut;

  // Valid color formats for the csc only case:
  //   1) if Vin or Vout are 422, and 422 or 420 are enabled, the case is allowed
  //   2) if Vin or Vout are 420, and 420 is enabled, the case is allowed

  if (((pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_422) ||
       (pStrmOut->ColorFormatId == XVIDC_CSF_YCRCB_422)) &&
      (!Allow422)) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_NO422);
    return(XST_FAILURE);
  }

  if (((pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420) ||
       (pStrmOut->ColorFormatId == XVIDC_CSF_YCRCB_420)) &&
      (!Allow420)) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_NO420);
    return(XST_FAILURE);
  }

  if(pStrmIn->VmId != pStrmOut->VmId) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_VMDIFF);
    return(XST_FAILURE);
  }
 if(pStrmIn->Timing.HActive != pStrmOut->Timing.HActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_HDIFF);
    return(XST_FAILURE);
  }

  if(pStrmIn->Timing.VActive != pStrmOut->Timing.VActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_VDIFF);
    return(XST_FAILURE);
  }

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_VALID);
  return(XST_SUCCESS);
}


static int ValidateVCResampleOnlyConfig(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoStream *pStrmIn  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *pStrmOut = &XVprocSsPtr->VidOut;

  if((pStrmIn->ColorFormatId != XVIDC_CSF_YCRCB_420) &&
         (pStrmIn->ColorFormatId != XVIDC_CSF_YCRCB_422)) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_CFIN);
    return(XST_FAILURE);
  }

  if((pStrmOut->ColorFormatId != XVIDC_CSF_YCRCB_420) &&
         (pStrmOut->ColorFormatId != XVIDC_CSF_YCRCB_422)) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_CFOUT);
    return(XST_FAILURE);
  }

  if(pStrmIn->VmId != pStrmOut->VmId) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_VMDIFF);
    return(XST_FAILURE);
  }

  if(pStrmIn->Timing.HActive != pStrmOut->Timing.HActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_HDIFF);
    return(XST_FAILURE);
  }

  if(pStrmIn->Timing.VActive != pStrmOut->Timing.VActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_VDIFF);
    return(XST_FAILURE);
  }
  return(XST_SUCCESS);
}


static int ValidateHCResampleOnlyConfig(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoStream *pStrmIn  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *pStrmOut = &XVprocSsPtr->VidOut;

  if((pStrmIn->ColorFormatId != XVIDC_CSF_YCRCB_422) &&
         (pStrmIn->ColorFormatId != XVIDC_CSF_YCRCB_444) ) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_CFIN);
    return(XST_FAILURE);
  }

  if((pStrmOut->ColorFormatId != XVIDC_CSF_YCRCB_422) &&
         (pStrmOut->ColorFormatId != XVIDC_CSF_YCRCB_444) ) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_CFOUT);
    return(XST_FAILURE);
  }

  if(pStrmIn->VmId != pStrmOut->VmId) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_VMDIFF);
    return(XST_FAILURE);
  }

  if(pStrmIn->Timing.HActive != pStrmOut->Timing.HActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_HDIFF);
    return(XST_FAILURE);
  }

  if(pStrmIn->Timing.VActive != pStrmOut->Timing.VActive) {
    //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_VDIFF);
    return(XST_FAILURE);
  } 
 return(XST_SUCCESS);
}


void XVprocSs_SetPictureColorStdIn(XVprocSs *InstancePtr,
                                       XVidC_ColorStd NewVal)
{
  XV_Csc_l2 *CscPtr = InstancePtr->CscPtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewVal >= XVIDC_BT_2020) &&
                         (NewVal < XVIDC_BT_NUM_SUPPORTED));
  
  if(InstancePtr->CscPtr) {
    XV_CscSetColorspace(CscPtr,
                        CscPtr->ColorFormatIn,
                        CscPtr->ColorFormatOut,
                        NewVal,
                        CscPtr->StandardOut,
                        CscPtr->OutputRange);
  }
}



void XVprocSs_SetPictureColorStdOut(XVprocSs *InstancePtr,
                                        XVidC_ColorStd NewVal)
{
  XV_Csc_l2 *CscPtr = InstancePtr->CscPtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewVal >= XVIDC_BT_2020) &&
                         (NewVal < XVIDC_BT_NUM_SUPPORTED));

  if(InstancePtr->CscPtr) {
    XV_CscSetColorspace(CscPtr,
                        CscPtr->ColorFormatIn,
                        CscPtr->ColorFormatOut,
                        CscPtr->StandardIn,
                        NewVal,
                        CscPtr->OutputRange);
  }
}


