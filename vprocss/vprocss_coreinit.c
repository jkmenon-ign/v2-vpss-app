
#include "vprocss_coreinit.h"


static int ComputeSubcoreAbsAddr(UINTPTR subsys_baseaddr,
                                         u32 subcore_offset,
                                                                 UINTPTR *subcore_baseaddr);

static int ComputeSubcoreAbsAddr(UINTPTR subsys_baseaddr,
                                         u32 subcore_offset,
                                            UINTPTR *subcore_baseaddr)
{
  int status;
  UINTPTR absAddr;

  absAddr = subsys_baseaddr + subcore_offset;

  if(absAddr>=subsys_baseaddr)
  {
    *subcore_baseaddr = absAddr;
    status = XST_SUCCESS;
  }
  else
  {
        *subcore_baseaddr = 0;
        status = XST_FAILURE;
  }

#if 0
  if((absAddr>=subsys_baseaddr) && (absAddr<subsys_highaddr))
  {
    *subcore_baseaddr = absAddr;
    status = XST_SUCCESS;
  }
  else
  {
        *subcore_baseaddr = 0;
        status = XST_FAILURE;
  }
#endif

  return(status);
}


int XVprocSs_SubcoreInitResetAxis(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XGpio_Config *pConfig;

  if(XVprocSsPtr->RstAxisPtr)
  {

    /* Get core configuration */
    pConfig  = XGpio_LookupConfig(XVprocSsPtr->Config.RstAxis.DeviceId);
    if(pConfig == NULL)
    {
      printf("Error: Reset_AxiS Config struct not found \n");
      //printf("%s: ERROR getting XGpio Config \n",__func__);
      return(XST_FAILURE);
    }

    
   // printf("%s \n", __func__);


    /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.RstAxis.AddrOffset,
                                           &AbsAddr);

    
    printf("absolute address of rstAxis : %lx \n", AbsAddr);
    
    /* Initialize core */
    status = XGpio_CfgInitialize(XVprocSsPtr->RstAxisPtr,
                                 pConfig,
                                 AbsAddr);

    if(status != XST_SUCCESS)
    {
      printf("reset axis failed \n");
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }
printf("Info: Reset_AxiS init \n");
  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}


int XVprocSs_SubcoreInitHCrsmplr(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_hcresampler_Config *pConfig;


  if(XVprocSsPtr->HcrsmplrPtr)
  { 
         /* Get core configuration */
    pConfig  = XV_hcresampler_LookupConfig(XVprocSsPtr->Config.HCrsmplr.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s: ERROR getting H-resampler config \n", __func__);
      return(XST_FAILURE);
    }
    
    /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.HCrsmplr.AddrOffset,
                                           &AbsAddr);
    
   printf("%s \n", __func__);
    
    if(status != XST_SUCCESS)
    {
      // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }
        
     /* Initialize core */
    status = XV_hcresampler_CfgInitialize(&XVprocSsPtr->HcrsmplrPtr->Hcr,
                                          pConfig,
                                          AbsAddr);
    if(status != XST_SUCCESS)
    {
      // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->HcrsmplrPtr->Hcr.Config.ResamplingType == XV_HCRSMPLR_TYPE_FIR)
    {
      printf("Loading default coefficients h-resampler \n");
      /* Load default filter coefficients */
      XV_HCrsmplLoadDefaultCoeff(XVprocSsPtr->HcrsmplrPtr);
          // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_LDCOEF);
    }
  }

  // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}



int XVprocSs_SubcoreInitHScaler(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_hscaler_Config *pConfig;
  

  if(XVprocSsPtr->HscalerPtr)
  {     
    /* Get core configuration */
    pConfig  = XV_hscaler_LookupConfig(XVprocSsPtr->Config.Hscale.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s:ERROR getting H-scaler config \n",__func__);
      return(XST_FAILURE);
    }

    
        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.Hscale.AddrOffset,
                                           &AbsAddr);
    
    if(status != XST_SUCCESS)
    {
      printf("Info: HScaler init, compute address failed \n"); 
      return(XST_FAILURE);
    }   
    
     /* Initialize core */
    status = XV_hscaler_CfgInitialize(&XVprocSsPtr->HscalerPtr->Hsc,
                                      pConfig,
                                      AbsAddr);
    
    if(status != XST_SUCCESS)
    { 
     
     printf("Info: HScaler init failed \n"); 
      return(XST_FAILURE);
    }
  }

  printf("Info: HScaler init \n");
  return(XST_SUCCESS);
}

/******************************************************************************/
int XVprocSs_SubcoreInitVScaler(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_vscaler_Config *pConfig;

   
  if(XVprocSsPtr->VscalerPtr)
  {
  
    pConfig  = XV_vscaler_LookupConfig(XVprocSsPtr->Config.Vscale.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s:ERROR getting V-scaler config \n",__func__);
      return(XST_FAILURE);
    }

        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.Vscale.AddrOffset,
                                           &AbsAddr);

    if(status != XST_SUCCESS)
    {

     printf("Info: VScaler init, config address failed  \n"); 
      return(XST_FAILURE);
    }


        /* Initialize core */
    status = XV_vscaler_CfgInitialize(&XVprocSsPtr->VscalerPtr->Vsc,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
       printf("Info: VScaler init failed \n");
      return(XST_FAILURE);
    }

  }

   
     printf("Info: VScaler init \n"); 

  return(XST_SUCCESS);
}


int XVprocSs_SubcoreInitRouter(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XAxis_Switch_Config *pConfig;

  /* Get core configuration */
  pConfig  = XAxisScr_LookupConfig(XVprocSsPtr->Config.Router.DeviceId);
  if(pConfig == NULL)
  {
    printf("%s: ERROR getting router config \n", __func__);
    return(XST_FAILURE);
  }
 
  if(XVprocSsPtr->RouterPtr)
  {

        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.Router.AddrOffset,
                                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
     // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }


        /* Initialize core */
    status = XAxisScr_CfgInitialize(XVprocSsPtr->RouterPtr,
                                    pConfig,
                                    AbsAddr);

    if(status != XST_SUCCESS)
    {
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}
  

int XVprocSs_SubcoreInitVCrsmpleIn(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_vcresampler_Config *pConfig;


  if(XVprocSsPtr->VcrsmplrInPtr)
  {

        /* Get core configuration */
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrIn.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s:ERROR getting v-resampler-in config \n", __func__);
      return(XST_FAILURE);
    }

        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.VCrsmplrIn.AddrOffset,
                                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }


        /* Initialize core */
    status = XV_vcresampler_CfgInitialize(&XVprocSsPtr->VcrsmplrInPtr->Vcr,
								 pConfig,
                                          			AbsAddr);

  if(status != XST_SUCCESS)
    {
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrInPtr->Vcr.Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      printf("loading Default coefficients V-resampler-In \n");
       XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrInPtr);
         // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_LDCOEF);
    }
  }

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}

int XVprocSs_SubcoreInitVCrsmpleOut(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_vcresampler_Config *pConfig;

  
  if(XVprocSsPtr->VcrsmplrOutPtr)
  {

    /* Get core configuration */
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrOut.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s:ERROR getting V-resampler out config \n", __func__);
      return(XST_FAILURE);
    }


        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.VCrsmplrOut.AddrOffset,
                                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }


        /* Initialize core */
    status = XV_vcresampler_CfgInitialize(&XVprocSsPtr->VcrsmplrOutPtr->Vcr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrOutPtr->Vcr.Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      printf("Loading Default coefficent V-resampler-Out \n");
      /* Load default filter coefficients */
      XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrOutPtr);
          //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_LDCOEF);
    }
  }

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}
                                                                                                              

int XVprocSs_SubcoreInitCsc(XVprocSs *XVprocSsPtr)
{
  int status;
  UINTPTR AbsAddr;
  XV_csc_Config *pConfig;

  if(XVprocSsPtr->CscPtr)
  {
        /* Get core configuration */
    pConfig  = XV_csc_LookupConfig(XVprocSsPtr->Config.Csc.DeviceId);
    if(pConfig == NULL)
    {
      printf("%s: ERROR getting csc config \n",__func__);
      return(XST_FAILURE);
    }

        /* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
                                           XVprocSsPtr->Config.Csc.AddrOffset,
                                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
     // XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }


        /* Initialize core */
    status = XV_csc_CfgInitialize(&XVprocSsPtr->CscPtr->Csc,
                                  pConfig,
                                  AbsAddr);

    if(status != XST_SUCCESS)
    {
    //  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  //XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}
                                                                                       
