/*******************************************************************************
 Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

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
#include "OpenCLIppSobelFilter.h"
#include "OpenclIppUtils.h"
#include <stdio.h>

static int sobelInitialized = 0;
static OpenclSobelSpec *sobelSpec;

OpenclIppStatus OpenCLSobel_Init(DeviceInfo *infoDeviceOcl,
                                            const OpenclIpp32f *input, 
                                            OpenclIpp32f *output,
                                            OpenclIppiSize dstRoiSize,                                                
                                            OpenclIpp32s filterSize,
                                            OpenclIpp32s bitWidth)
{
    int err;
    
    if (sobelInitialized)
        return true;

    int inputWidth = dstRoiSize.width;
    int inputHeight = dstRoiSize.height;

    if ((err = OpenCLIppInit()) == false)
    {
        return err;
    }

    sobelSpec = (OpenclSobelSpec *)malloc(sizeof(OpenclSobelSpec));
    if (!sobelSpec)
    {
        return false;
    }

    /* Create cl_mem */

    sobelSpec->inputWidth = inputWidth;
    sobelSpec->inputHeight = inputHeight;

    sobelSpec->input = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
                    inputWidth * inputHeight * (bitWidth / 8), (void *)input, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    sobelSpec->output = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,
                    inputWidth * inputHeight * (bitWidth / 8), output, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    float *verPass1Filter, *verPass2Filter, *horPass1Filter, *horPass2Filter;
    if (filterSize == 5)
	{
		
		verPass1Filter = SOBEL_FILTER_VER_5x5_pass1;
		verPass2Filter = SOBEL_FILTER_VER_5x5_pass2;		
		horPass1Filter = SOBEL_FILTER_HORIZ_5x5_pass1;
		horPass2Filter = SOBEL_FILTER_HORIZ_5x5_pass2;
		
	}
	else if (filterSize == 3)
	{
		verPass1Filter = SOBEL_FILTER_VER_3x3_pass1;
		verPass2Filter = SOBEL_FILTER_VER_3x3_pass2;
		horPass1Filter = SOBEL_FILTER_HORIZ_3x3_pass1;
		horPass2Filter = SOBEL_FILTER_HORIZ_3x3_pass2;
		
	}
    else {
        free(sobelSpec);
        return err;
           
    }

    sobelSpec->verPass1 = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(float), verPass1Filter, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    sobelSpec->verPass2 = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(float), verPass2Filter, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }
    
    sobelSpec->horPass1 = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(float), horPass1Filter, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    sobelSpec->horPass2 = clCreateBuffer(infoDeviceOcl->mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(float), horPass2Filter, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    /* Build the separable sobel kernel */
    cl_context oclCtx=infoDeviceOcl->mCtx;
	cl_device_id oclDevice=infoDeviceOcl->mDevice;
    const char *filename = SEPARABLE_KERNEL_SOURCE;

    char *source;
    size_t sourceSize;
    err = convertToString(filename, &source, &sourceSize);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    cl_program program;

    program = clCreateProgramWithSource(oclCtx, 1, (const char **) &source,
                    &sourceSize, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    /*************************************************************************
     * Build the kernel and check for errors. If errors are found, it will be  *
     * dumped into buildlog.txt                                                *
     **************************************************************************/
    char option[256];
    int useLds = 0;
	sprintf(option, "-DPIX_WIDTH=%d -DFILTERSIZE=%d -DLOCAL_XRES=%d -DLOCAL_YRES=%d -DUSE_LDS=%d",
                    bitWidth, filterSize, LOCAL_XRES, LOCAL_YRES, useLds);

    err = clBuildProgram(program, 1, &oclDevice, option, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t length;

        int size = 4096;
        char *buffer = (char *) malloc(size);
        
        clGetProgramBuildInfo(program, oclDevice, CL_PROGRAM_BUILD_LOG, size,
                        buffer, &length);

        FILE *fp;
        fp = fopen("buildlog.txt", "w");
        fprintf(fp, "%s\n", buffer);
        fclose(fp);
        free(buffer);

        clReleaseContext(oclCtx);
        return false;
    }

    sobelSpec->separableKernel = clCreateKernel(program, SEPARABLE_FILTER_KERNEL, &err);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

	clReleaseProgram(program);

    /* Set kernel arg */
    int cnt = 0;
    
    err  = clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->input);
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->verPass1);
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->verPass2);
	err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->horPass1);
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->horPass2);    
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_mem), &sobelSpec->output);
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_int), &inputWidth);
	err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_int), &inputHeight);
    err |= clSetKernelArg(sobelSpec->separableKernel, cnt++, sizeof(cl_int), &inputWidth);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    sobelInitialized = 1;
    
    return true;
}

OpenclIppStatus OpenCLSobel_Run(DeviceInfo *infoDeviceOcl)
{
    OpenclIppStatus err;
	
    /******************************************************************************
     * Enqueue kernel
     ******************************************************************************/
    size_t localWorkSize[2] = { LOCAL_XRES, LOCAL_YRES };
    size_t globalWorkSize[2];

    globalWorkSize[0] = (sobelSpec->inputWidth + localWorkSize[0] - 1) / localWorkSize[0];
    globalWorkSize[0] *= localWorkSize[0];
    globalWorkSize[1] = (sobelSpec->inputHeight + localWorkSize[1] - 1) / localWorkSize[1];
    globalWorkSize[1] *= localWorkSize[1];

    err = clEnqueueNDRangeKernel(infoDeviceOcl->mQueue, sobelSpec->separableKernel, 2, NULL,
                    globalWorkSize, localWorkSize, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        free(sobelSpec);
        return err;
    }

    clFinish(infoDeviceOcl->mQueue);
    return true;
}

void OpenCLSobelDeInit()
{
    if (sobelSpec)
    {
        clReleaseMemObject(sobelSpec->verPass1);
        clReleaseMemObject(sobelSpec->verPass2);
        clReleaseMemObject(sobelSpec->horPass1);
	    clReleaseMemObject(sobelSpec->horPass2);
        clReleaseMemObject(sobelSpec->input);
        clReleaseMemObject(sobelSpec->output);
        clReleaseKernel(sobelSpec->separableKernel);

        free(sobelSpec);

        sobelInitialized = 0;
    }
}