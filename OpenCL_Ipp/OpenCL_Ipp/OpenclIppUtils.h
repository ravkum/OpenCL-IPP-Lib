#ifndef _OPENCLIPPUTILS_H_
#define _OPENCLIPPUTILS_H_

#include "CL/cl.h"
#include <stdlib.h>

/******************************************************************************
* Error handling macro.                                                       *
******************************************************************************/
#ifdef _MSC_VER
#define snprintf sprintf_s
#endif
#define CHECK_RESULT(test, msg,...)                                     \
    if ((test))                                                         \
    {                                                                   \
        char *buf = (char*)malloc(4096);                                \
        int rc = snprintf(buf, 4096, msg,  ##__VA_ARGS__);              \
        printf("%s:%d - %s\n", __FILE__, __LINE__, buf);                \
        free(buf);                                                      \
        return false;                                                   \
    }   

typedef struct DeviceInfo
{
    cl_platform_id mPlatform;
    cl_device_id mDevice;
    cl_context mCtx;
    cl_command_queue mQueue;
} DeviceInfo;


int initOpenCl(DeviceInfo *infoDeviceOcl, cl_uint deviceNum);
int convertToString(const char *filename, char **kernelSource, size_t *kernelSourceSize);

#endif