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
/**
 ********************************************************************************
 * @file <separableFilter.cpp>
 *
 * @brief This file implements functions related to separable filter openCL kernel.
 *
 ********************************************************************************
 */

#include"separableFilter.h"
#ifdef PERFORMANCE_READ_WRITE
	#define CLEVENT(X)	X
#else
	#define CLEVENT(X)	NULL
#endif
/**
 *******************************************************************************
 *  @fn     init
 *  @brief  This function parses creates the memory needed by the pipeine, 
 *          initializes openCL, builds kernel and sets the kernel arguments
 *
 *  @param[in] filterSize       : filter size (only 3 and 5 are currently supported)
 *  @param[in] bitWidth         : 8 bit or 16 bit input
 *  @param[in] deviceNum        : device on which to run OpenCL kernels
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool SeperableFilter::init(cl_int filterSize,cl_uint bitWidth, cl_uint deviceNum, cl_int useLds,cl_int width,cl_int height)
{
	this->bitWidth=bitWidth; 
	this->rows=height;
	this->cols=width;
	this->filterSize=filterSize;
	this->useLds=useLds;
	this->isFirstIteration=true;
	this->paddedCols=this->cols;
	this->paddedRows=this->rows;
	

	
    /**************************************************************************
     * Initialize the openCL device and create context and command queue       *
     ***************************************************************************/
    if (initOpenCl(&infoDeviceOcl, deviceNum) == false)
    {
        printf("Error in initOpenCl.\n");
        return false;
    }

    /**************************************************************************
     * Create the memory needed by the pipeline                                *
     ***************************************************************************/
    if (createMemory() == false)
    {
        printf("Error in createMemory.\n");
        return false;
    }

    /***************************************************************************
     * Build the separable Filter OpenCL kernel                                      *
     ***************************************************************************/
    if (buildSeparableFilterKernel()== false)
    {
        printf("Error in buildKernelFilter.\n");
        return false;
    }
    return true;
}

bool SeperableFilter::createMemory()
{
    cl_int err = 0;
	cl_int filterRadius = filterSize / 2;
    /***********************************************************************
     * get filter 
     ***********************************************************************/    
	if (filterSize == 5)
	{
		
		rowFilterCpu = SOBEL_FILTER_VER_5x5_pass1;
		colFilterCpu = SOBEL_FILTER_VER_5x5_pass2;		
		rowFilterCpu1 = SOBEL_FILTER_HORIZ_5x5_pass1;
		colFilterCpu1 = SOBEL_FILTER_HORIZ_5x5_pass2;
		
	}
	else
	{
		rowFilterCpu = SOBEL_FILTER_VER_3x3_pass1;
		colFilterCpu = SOBEL_FILTER_VER_3x3_pass2;
		rowFilterCpu1 = SOBEL_FILTER_HORIZ_3x3_pass1;
		colFilterCpu1 = SOBEL_FILTER_HORIZ_3x3_pass2;
		
	}
	
	rowFilter = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(cl_float), rowFilterCpu, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);

    colFilter = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(cl_float), colFilterCpu, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);
	rowFilter1 = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(cl_float), rowFilterCpu1, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);

    colFilter1 = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                    filterSize * sizeof(cl_float), colFilterCpu1, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);
#if 0
	/*outputImg = (cl_uchar *) malloc((paddedRows) * cols* sizeof(float));
    CHECK_RESULT(outputImg == NULL, "Malloc failed.\n");
	
	output = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, paddedRows * cols * sizeof(float), outputImg, &err);
	CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);*/
	input = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_READ_ONLY, paddedRows * cols * sizeof(float), NULL, &err);
	CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);
	output = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_WRITE_ONLY, paddedRows * cols * sizeof(float), NULL, &err);
	CHECK_RESULT(err != CL_SUCCESS, "clCreateBuffer failed with %d\n", err);
#endif
	
    return true;
}
/**
 *******************************************************************************
 *  @fn     buildKernelFilter
 *  @brief  This functons builds the separableFilter OpenCL kernel and also sets the 
 *          kernel arguments.
 *
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool SeperableFilter::buildSeparableFilterKernel()
{
	cl_context oclCtx=infoDeviceOcl.mCtx;
	cl_device_id oclDevice=infoDeviceOcl.mDevice;
    const char *filename = KERNEL_SOURCE;
    cl_int err;
    char *source;
    size_t sourceSize;
    err = convertToString(filename, &source, &sourceSize);
    CHECK_RESULT(err != CL_SUCCESS, "Error reading file %s ", filename);

    cl_program program;

    program = clCreateProgramWithSource(oclCtx, 1, (const char **) &source,
                    &sourceSize, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateProgram failed with %d\n", err);

    /*************************************************************************
     * Build the kernel and check for errors. If errors are found, it will be  *
     * dumped into buildlog.txt                                                *
     **************************************************************************/
    char option[256];
	sprintf(option, "-DPIX_WIDTH=%d -DFILTERSIZE=%d -DLOCAL_XRES=%d -DLOCAL_YRES=%d -DUSE_LDS=%d",
                    bitWidth, filterSize, LOCAL_XRES, LOCAL_YRES, useLds);

    err = clBuildProgram(program, 1, &oclDevice, option, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("clCreateProgram failed with %d\n", err);

        printf("Program Build failed\n");
        size_t length;

        int size = 4096;
        char *buffer = (char *) malloc(size);
        printf("sizeof buf: %d\n", sizeof(buffer));

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

    separableKernel = clCreateKernel(program, SEPARABLE_FILTER_KERNEL, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateKernel failed with %d\n", err);
	clReleaseProgram(program);
    return true;
}


/**
 *******************************************************************************
 *  @fn     destroyMemory
 *  @brief  This function destroys the memory created by the pipeline
 *
 *  @param[in/out] paramFF  : pointer to structure
 *  @param[in] infoDeviceOcl   : pointer to the structure containing opencl
 *                               device information
 *
 *  @return void
 *******************************************************************************
 */
void SeperableFilter::destroyMemory()
{
	clReleaseCommandQueue(infoDeviceOcl.mQueue);
	clReleaseContext(infoDeviceOcl.mCtx);
	clReleaseDevice(infoDeviceOcl.mDevice);
	clReleaseMemObject(input);
    clReleaseMemObject(rowFilter);
    clReleaseMemObject(colFilter);
	clReleaseMemObject(rowFilter1);
    clReleaseMemObject(colFilter1);
    clReleaseMemObject(output);
    clReleaseKernel(separableKernel);
}
/**
 *******************************************************************************
 *  @fn     run
 *  @brief  This function runs the entire pipeline
 *
 *  @param[in] inputImage : inputImage
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool SeperableFilter::run(LOG_DATA_TYPE *inputImage, LOG_DATA_TYPE *outputImage)
{
	cl_int status = 0;
	cl_uint dataSize=bitWidth/8;	
	
	if(isFirstIteration)
	{
		
		input = clCreateBuffer( infoDeviceOcl.mCtx, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,  paddedRows * paddedCols * sizeof(LOG_DATA_TYPE) , (void *)inputImage, &status );
		CHECK_RESULT(status != CL_SUCCESS, "clCreateBuffer failed with %d\n", status);
		output = clCreateBuffer(infoDeviceOcl.mCtx, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, paddedRows * cols * sizeof(LOG_DATA_TYPE), outputImage, &status);
		CHECK_RESULT(status != CL_SUCCESS, "clCreateBuffer failed with %d\n", status);
		isFirstIteration=false;	
        /***************************************************************************
        * Set the pass1 Filter OpenCL kernel arguments                             *
        ***************************************************************************/
	
        if (setSeparableFilterKernelArgs() == false)
	    {
		    printf("Error in setFreqFilterKernelArgs.\n");
		    return false;
	    }	
	}

    runSeparableFilterKernel();
	clFinish(infoDeviceOcl.mQueue);	
	return true;
}



/**
 **************************************************************************************
 *  @fn     setPass1FilterKernelArgs
 *  @brief  This functons sets the argument of the Pass1Filter OpenCL kernel.
 *
 *
 *  @return bool : true if successful; otherwise false.
 **************************************************************************************
 */
bool SeperableFilter::setSeparableFilterKernelArgs()
{

    cl_int cnt = 0;
    cl_int err;
	int rows1=((rows+LOCAL_YRES-1)/16)*16;
    err = clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem),  &input);
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem), &rowFilter);
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem), &colFilter);
	err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem), &rowFilter1);
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem), &colFilter1);    
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_mem), &output);
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_int), &cols);
	err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_int), &rows);
    err |= clSetKernelArg(separableKernel, cnt++, sizeof(cl_int), &paddedCols);

    CHECK_RESULT(err != CL_SUCCESS, "clSetKernelArg failed with %d\n", err);

    return true;
}

/**
 *******************************************************************************
 *  @fn     runseparableFilterKernel
 *  @brief  This function enqueues the separableFilter kernel on the OpenCL device
 *
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool SeperableFilter::runSeparableFilterKernel()
{
    cl_int err;
	cl_command_queue oclQueue = infoDeviceOcl.mQueue;
    

    /******************************************************************************
     * Enqueue pass1 kernel
     ******************************************************************************/
    size_t localWorkSize[2] = { LOCAL_XRES, LOCAL_YRES };
    size_t globalWorkSize[2];

    globalWorkSize[0] = (cols + localWorkSize[0] - 1) / localWorkSize[0];
    globalWorkSize[0] *= localWorkSize[0];
    globalWorkSize[1] = (rows + localWorkSize[1] - 1) / localWorkSize[1];
    globalWorkSize[1] *= localWorkSize[1];

    err = clEnqueueNDRangeKernel(oclQueue, separableKernel, 2, NULL,
                    globalWorkSize, localWorkSize, 0, NULL, NULL);
    CHECK_RESULT(err != CL_SUCCESS, "clEnqueueNDRangeKernel failed with %d\n",
                    err);

    return true;

}

SeperableFilter::SeperableFilter()
{

}
SeperableFilter::~SeperableFilter()
{
	destroyMemory();
}
