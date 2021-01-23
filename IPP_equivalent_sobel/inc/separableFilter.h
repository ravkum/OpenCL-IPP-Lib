/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
#ifndef __SEPARABLEFILTER__H
#define __SEPARABLEFILTER__H
#define LOG_DATA_TYPE float
#include "macros.h"
#include "utils.h"
#include "CL/cl.h"

#define LOCAL_XRES  16
#define LOCAL_YRES  16

#define KERNEL_SOURCE	                "separableFilter.cl"
#define SEPARABLE_FILTER_KERNEL	        "separableFilter"
#define PASS2_SEPARABLE_FILTER_KERNEL	"pass2SeparableFilter"
#if 0
static float BOX_FILTER_5x5_pass1[5] = { -0.20000, -0.20000, -0.20000, -0.20000,-0.20000 };
static float BOX_FILTER_5x5_pass2[5] = { -0.20000, -0.20000, -0.20000, -0.20000,-0.20000 };
static float BOX_FILTER_3x3_pass1[3] = { -0.33166, -0.33166, -0.33166 };
static float BOX_FILTER_3x3_pass2[3] = { -0.33166, -0.33166, -0.33166 };
#else
static float SOBEL_FILTER_VER_5x5_pass1[5] = { 1, 4, 6, 4, 1 };
static float SOBEL_FILTER_VER_5x5_pass2[5] = { -1, -2, 0, 2, 1 };
static float SOBEL_FILTER_VER_3x3_pass1[3] = { 1, 2, 1 };
static float SOBEL_FILTER_VER_3x3_pass2[3] = { -1,0,1 };

static float SOBEL_FILTER_HORIZ_5x5_pass2[5] = { 1, 4, 6, 4, 1 };
static float SOBEL_FILTER_HORIZ_5x5_pass1[5] = { 1, 2, 0, -2, -1 };
static float SOBEL_FILTER_HORIZ_3x3_pass2[3] = { 1, 2, 1 };
static float SOBEL_FILTER_HORIZ_3x3_pass1[3] = { 1,0,-1 };
#endif

class SeperableFilter{

public:
	bool init(cl_int filterSize, cl_uint bitWidth,
				cl_uint deviceNum, cl_int useLds,
				cl_int width, cl_int height);	

	bool run(LOG_DATA_TYPE *in,LOG_DATA_TYPE *out);
	
	SeperableFilter();
	~SeperableFilter();
private:
	DeviceInfo infoDeviceOcl;
	cl_uint rows;
    cl_uint cols;

    cl_uint paddedRows;
    cl_uint paddedCols;

    cl_uint filterSize;
    cl_float *rowFilterCpu;
    cl_float *colFilterCpu;
	cl_float *rowFilterCpu1;
    cl_float *colFilterCpu1;

    cl_uchar *inputImg;
    cl_uchar *outputImg;

    cl_mem input;
    cl_mem rowFilter;
    cl_mem colFilter;
	cl_mem rowFilter1;
    cl_mem colFilter1;
    cl_mem output;

    cl_kernel separableKernel;

	cl_uint bitWidth;
    cl_uint useLds;
	

	bool createMemory();
	void destroyMemory();
	bool buildSeparableFilterKernel();
	bool setSeparableFilterKernelArgs();	
	bool runSeparableFilterKernel(/*cl_int width, cl_int height*/);
	bool isFirstIteration;
};

#endif  

