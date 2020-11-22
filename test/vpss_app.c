#include "vprocss.h"


/************************** Local Typedefs **********************************/
typedef struct {
  u16 width_in;
  u16 height_in;
  XVidC_ColorFormat Cformat_in;
  u16 IsInterlaced;

  u16 width_out;
  u16 height_out;
  XVidC_ColorFormat Cformat_out;
} vpssVideo;

/************************** Constant Definitions *****************************/
typedef enum
{
  XSYS_VPSS_STREAM_IN = 0,
  XSYS_VPSS_STREAM_OUT
}XSys_StreamDirection;



XVprocSs VprocInst;
vpssVideo UseCase = {1920, 1080, XVIDC_CSF_YCRCB_422, FALSE, 
     3840, 2160, XVIDC_CSF_YCRCB_422};

static void check_usecase(XVprocSs *VpssPtr, vpssVideo *useCase)
{
  u16 *width_in   = &useCase->width_in;
  u16 *width_out  = &useCase->width_out;
  u16 *height_in  = &useCase->height_in;
  u16 *height_out = &useCase->height_out;

 printf("checking use cases........ \n");
    // check/correct Max Width
    if(*width_in > VpssPtr->Config.MaxWidth)
      *width_in = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_in % VpssPtr->Config.PixPerClock) != 0)
    *width_in -= (*width_in % VpssPtr->Config.PixPerClock);

  // check/correct Max Width
  if(*width_out > VpssPtr->Config.MaxWidth)
    *width_out = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_out % VpssPtr->Config.PixPerClock) != 0)
    *width_out -= (*width_out % VpssPtr->Config.PixPerClock);

  // check/correct Max Height
  if(*height_in > VpssPtr->Config.MaxHeight)
    *height_in = VpssPtr->Config.MaxHeight;

  // check/correct Max Height
  if(*height_out > VpssPtr->Config.MaxHeight)
    *height_out = VpssPtr->Config.MaxHeight;
}

int XSys_SetStreamParam(XVprocSs *pVprocss, u16 Direction, u16 Width,
                        u16 Height, XVidC_FrameRate FrameRate,
                        XVidC_ColorFormat cfmt, u16 IsInterlaced)
{
        printf("setting I/O stream parameters \n");
        XVidC_VideoMode resId;
        XVidC_VideoStream Stream;
        XVidC_VideoTiming const *TimingPtr;

        resId = XVidC_GetVideoModeId(Width, Height, FrameRate, IsInterlaced);
        if (resId == XVIDC_VM_NOT_SUPPORTED)
                return XST_INVALID_PARAM;

        TimingPtr = XVidC_GetTimingInfo(resId);

        /* Setup Video Processing Subsystem */
        Stream.VmId           = resId;
        Stream.Timing         = *TimingPtr;
        Stream.ColorFormatId  = cfmt;
        Stream.ColorDepth     = pVprocss->Config.ColorDepth;
        Stream.PixPerClk      = pVprocss->Config.PixPerClock;
        Stream.FrameRate      = FrameRate;
        Stream.IsInterlaced   = IsInterlaced;

        if (Direction == XSYS_VPSS_STREAM_IN)
                XVprocSs_SetVidStreamIn(pVprocss, &Stream);
        else
                XVprocSs_SetVidStreamOut(pVprocss, &Stream);

        return XST_SUCCESS;
}


int main(void)
{
 XVprocSs *VpssPtr;
 vpssVideo *thisCase; 
 int status;
 
 thisCase = &UseCase;

 VpssPtr = (XVprocSs *)malloc(sizeof(XVprocSs));

 VpssPtr = &VprocInst;

 VprocSs_Initialize(VpssPtr, "v_proc_ss");
 
 check_usecase(VpssPtr, thisCase);
 

 /*
  * Set VPSS Video Input AXI Stream to match the TPG
  * Note that framerate is hardwired to 60Hz in the example design
  */
 status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_IN,
		 thisCase->width_in,
		 thisCase->height_in, XVIDC_FR_60HZ,
		 thisCase->Cformat_in,
		 thisCase->IsInterlaced);
 if (status != XST_SUCCESS){
	 printf("Failed setting input stream parameters \n");
	 return XST_FAILURE;
 }
 /*
  * Set VPSS Video Output AXI Stream
  * Note that output video is always progressive
  */
 status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_OUT,
		 thisCase->width_out, thisCase->height_out,
		 XVIDC_FR_60HZ, thisCase->Cformat_out,
		 FALSE);
 if (status != XST_SUCCESS){
	 printf("Failed setting Output parameters \n");
	 return XST_FAILURE;
 }


 status = XVprocSs_SetSubsystemConfig(VpssPtr);
 if (status != XST_SUCCESS){
	 printf("Failed setting subsystem config parameters \n");
	 return XST_FAILURE;
 }

 XVprocSs_ReportSubcoreStatus(VpssPtr,XVPROCSS_SUBCORE_SCALER_V);

 XVprocSs_ReportSubcoreStatus(VpssPtr,XVPROCSS_SUBCORE_SCALER_H);

 XVprocSs_ReportSubsystemConfig(VpssPtr);
 
 return status;
}
