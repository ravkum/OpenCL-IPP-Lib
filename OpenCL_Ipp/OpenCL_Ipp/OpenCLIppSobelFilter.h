#ifndef _OPENCL_IPP_SOBEL_FILTER_H_
#define _OPENCL_IPP_SOBEL_FILTER_H_

#include "CL/cl.h"
#include "OpenclIpp.h"
#include "OpenclIppUtils.h"

#define LOCAL_XRES  16
#define LOCAL_YRES  16

#define SEPARABLE_KERNEL_SOURCE         "separableFilter.cl"
#define SEPARABLE_FILTER_KERNEL	        "separableFilter"

static float SOBEL_FILTER_VER_5x5_pass1[5] = { 1, 4, 6, 4, 1 };
static float SOBEL_FILTER_VER_5x5_pass2[5] = { -1, -2, 0, 2, 1 };
static float SOBEL_FILTER_VER_3x3_pass1[3] = { 1, 2, 1 };
static float SOBEL_FILTER_VER_3x3_pass2[3] = { -1,0,1 };

static float SOBEL_FILTER_HORIZ_5x5_pass2[5] = { 1, 4, 6, 4, 1 };
static float SOBEL_FILTER_HORIZ_5x5_pass1[5] = { 1, 2, 0, -2, -1 };
static float SOBEL_FILTER_HORIZ_3x3_pass2[3] = { 1, 2, 1 };
static float SOBEL_FILTER_HORIZ_3x3_pass1[3] = { 1,0,-1 };


struct OpenclSobelSpec 
{
    cl_mem input;
    cl_mem output;

    cl_mem verPass1;
    cl_mem verPass2;
	cl_mem horPass1;
    cl_mem horPass2;

    int inputWidth;
    int inputHeight;
    
    cl_kernel separableKernel;
};

OpenclIppStatus OpenCLSobel_Init(DeviceInfo *infoDeviceOcl,
                                            const OpenclIpp32f *input, 
                                            OpenclIpp32f *output,
                                            OpenclIppiSize dstRoiSize,                                                
                                            OpenclIpp32s filterSize,
                                            OpenclIpp32s bitWidth);

OpenclIppStatus OpenCLSobel_Run(DeviceInfo *infoDeviceOcl);

void OpenCLSobelDeInit();

#endif