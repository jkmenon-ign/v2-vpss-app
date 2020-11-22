#ifndef XVPROCSS_COREINIT_H__  /* prevent circular inclusions */
#define XVPROCSS_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "vprocss.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XVprocSs_SubcoreInitResetAxis(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitRouter(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitHScaler(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVScaler(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitHCrsmplr(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVCrsmpleIn(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitVCrsmpleOut(XVprocSs *XVprocSsPtr);
int XVprocSs_SubcoreInitCsc(XVprocSs *XVprocSsPtr);
#ifdef __cplusplus
}
#endif

#endif




