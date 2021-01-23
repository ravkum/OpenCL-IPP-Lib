#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OpenclIppUtils.h"
                                                                

/**
*******************************************************************************
*  @fn     initOpenCl
*  @brief  This function creates the opencl context and command queue
*
*  @param[in/out] infoDeviceOcl  : pointer to structure
*  @param[in] deviceNum          : pointer to the structure containing opencl
*                                  device information
*
*  @return bool : true if successful; otherwise false.
*******************************************************************************
*/
int initOpenCl(DeviceInfo *infoDeviceOcl, cl_uint deviceNum)
{
    cl_int err;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = infoDeviceOcl->mCtx;
    cl_command_queue queue = 0;
    cl_event event = NULL;
    
    /**************************************************************************
    * Setup OpenCL environment.                                               *
    **************************************************************************/
    /* Get numPlatforms */
    cl_uint numPlatforms = 0;    
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    CHECK_RESULT(numPlatforms == 0, "clGetPlatformIDs failed. Error code = %d", err);

    /* Get AMD platform */
    cl_platform_id platform = NULL;
    for (cl_uint i = 1; i < numPlatforms+1; i++)
    {
        size_t param_size = 0;
        char *platformVendor = NULL;
        err = clGetPlatformIDs(i, &platform, NULL);
        CHECK_RESULT(err != CL_SUCCESS, "clGetPlatformIDs failed. Error code = %d", err);
        err = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, NULL, &param_size);
        CHECK_RESULT(err != CL_SUCCESS, "clGetPlatformInfo failed. Error code = %d", err);
        platformVendor = (char*)malloc(param_size);
        CHECK_RESULT(platformVendor == NULL, "Memory allocation failed: platformVendor");
        err = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, param_size, platformVendor, NULL);
        CHECK_RESULT(err != CL_SUCCESS, "clGetPlatformInfo failed. Error code = %d", err);
        bool match = (strstr(platformVendor, "Advanced Micro Devices, Inc.") != NULL);
        free(platformVendor);
        if (match)
            break;
        else
            platform = NULL;
    }
    CHECK_RESULT(platform == NULL, "AMD platform not found");
    infoDeviceOcl->mPlatform = platform;

#ifdef USE_OPENCL_CPU
    /* Get first CPU on platform */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &infoDeviceOcl->mDevice, 0);
    CHECK_RESULT(err != CL_SUCCESS, "clGetDeviceIDs failed. Error code = %d", err);
#else
    /* Get num GPUs on platform */
    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
    CHECK_RESULT(err != CL_SUCCESS, "clGetDeviceIDs failed. Error code = %d", err);
    CHECK_RESULT(numDevices < 1, "Atleast one GPU required for the test");
    CHECK_RESULT(numDevices <= deviceNum, "Requested device exceeds available devices (%d).", numDevices);

    /* Get list of GPUs on platform */
    cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id)*numDevices);
    CHECK_RESULT(devices == NULL, "Memory allocation failed: devices");
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, 0);
    CHECK_RESULT(err != CL_SUCCESS, "clGetDeviceIDs failed. Error code = %d", err);
    infoDeviceOcl->mDevice = devices[deviceNum];
    free(devices);
#endif

    props[1] = (cl_context_properties)platform;
    infoDeviceOcl->mCtx = clCreateContext(props, 1, &infoDeviceOcl->mDevice, NULL, NULL, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateContext failed. Err code = %d", err);

    infoDeviceOcl->mQueue = clCreateCommandQueue(infoDeviceOcl->mCtx, infoDeviceOcl->mDevice, 0, &err);
    CHECK_RESULT(err != CL_SUCCESS, "clCreateCommandQueue failed. Err code = %d", err);
    return true;
}

/**
*******************************************************************************
*  @fn     convertToString
*  @brief  convert the kernel file into a string
*
*  @param[in] filename          : Kernel file name to read
*  @param[out] kernelSource     : Buffer containing the contents of kernel file
*  @param[out] kernelSourceSize : Size of the buffer containing the source
*
*  @return int : 0 if successful; otherwise 1.
*******************************************************************************
*/
int convertToString(const char *filename, char **kernelSource, size_t *kernelSourceSize)
{
    FILE *fp = NULL;
    *kernelSourceSize = 0;
    *kernelSource = NULL;
    fp = fopen(filename, "r");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        *kernelSourceSize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        *kernelSource = (char*)malloc(*kernelSourceSize);
        if (*kernelSource != NULL)
            *kernelSourceSize = fread(*kernelSource, 1, *kernelSourceSize, fp);
        fclose(fp);
        return false;
    }

    return true;
}