// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

/***************************** Include Files *********************************/
#include "vprocss.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
typedef struct {
    u32 addr;
    u32 size;
} VprocSs_uio_map;

typedef struct {
    int  uio_fd;
    int  uio_num;
    char name[ MAX_UIO_NAME_SIZE ];
    char version[ MAX_UIO_NAME_SIZE ];
    VprocSs_uio_map maps[ MAX_UIO_MAPS ];
} VprocSs_uio_info;

/***************** Variable Definitions **************************************/
static VprocSs_uio_info uio_info;

/************************** Function Implementation *************************/
static int line_from_file(char* filename, char* linebuf) {
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    s = fgets(linebuf, MAX_UIO_NAME_SIZE, fp);
    fclose(fp);
    if (!s) return -2;
    for (i=0; (*s)&&(i<MAX_UIO_NAME_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return 0;
}

static int uio_info_read_name(VprocSs_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

static int uio_info_read_version(VprocSs_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

static int uio_info_read_map_addr(VprocSs_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    info->maps[n].addr = UIO_INVALID_ADDR;
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/addr", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].addr);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

static int uio_info_read_map_size(VprocSs_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/size", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].size);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

int VprocSs_Initialize(XVprocSs *InstancePtr, const char* InstanceName) {
    VprocSs_uio_info *InfoPtr = &uio_info;
    struct dirent **namelist;
    //int i; 
    int n;
    //char* s;
    char file[ MAX_UIO_PATH_SIZE ];
    //char name[ MAX_UIO_NAME_SIZE ];
    int flag = 0;

    XVprocSs_Config *VprocSsConfigPtr;
    assert(InstancePtr != NULL);
    printf("instance name %s \n",InstanceName);

    n = scandir("/sys/class/uio", &namelist, 0, alphasort);
    if (n < 0)  return XST_DEVICE_NOT_FOUND;
#if 0    
    for (i = 0;  i < n; i++) {
    strcpy(file, "/sys/class/uio/");
    strcat(file, namelist[i]->d_name);
    strcat(file, "/name");
        if ((line_from_file(file, name) == 0) && (strcmp(name, InstanceName) == 0)) {
            flag = 1;
            s = namelist[i]->d_name;
            s += 3; // "uio"
            InfoPtr->uio_num = atoi(s);
            break;
        }
    }

#endif    
    flag = 1;
    if (flag == 0)  return XST_DEVICE_NOT_FOUND;

    InfoPtr->uio_num = 4;
    uio_info_read_name(InfoPtr);
    uio_info_read_version(InfoPtr);
    for (n = 0; n < MAX_UIO_MAPS; ++n) {
        uio_info_read_map_addr(InfoPtr, n);
        uio_info_read_map_size(InfoPtr, n);
    }

    sprintf(file, "/dev/uio%d", InfoPtr->uio_num);
    if ((InfoPtr->uio_fd = open(file, O_RDWR)) < 0) {
        return XST_OPEN_DEVICE_FAILED;
    }
    
    VprocSsConfigPtr = XVprocSs_LookupConfig(XPAR_HDMI_PATH_V_PROC_SS_0_DEVICE_ID);
 
    if(VprocSsConfigPtr == NULL)
    {
       printf("ERR:: VprocSs device not found\r\n");
       return (XST_DEVICE_NOT_FOUND);
    }
    // NOTE: slave interface 'Ctrl' should be mapped to uioX/map0
    VprocSsConfigPtr->BaseAddress = (UINTPTR)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    //assert(InstancePtr->Config.BaseAddress);

     printf("vpss mapped address is %lx , size: %x \n", VprocSsConfigPtr->BaseAddress, InfoPtr->maps[0].size);
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
 
    XVprocSs_CfgInitialize(InstancePtr, VprocSsConfigPtr, VprocSsConfigPtr->BaseAddress);
  
    return XST_SUCCESS;
}

int VprocSs_Release(XVprocSs *InstancePtr) {
    VprocSs_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
