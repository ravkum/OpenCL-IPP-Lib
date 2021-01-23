// Opencl_ipp.cpp : Defines the exported functions for the DLL application.
//

#define OPENCL_IPP_API_EXPORTS
#include "OpenclIpp.h"
#include "OpenclIppUtils.h"
#include "OpenCLIppSobelFilter.h"

static DeviceInfo infoDeviceOcl;
int OpenCLInitialized = 0;

OPENCL_IPP_API int OpenCLIppInit()
{
    if (!OpenCLInitialized)
    {
        if (initOpenCl(&infoDeviceOcl, 0) == false)
        {
            return false;
        }

        OpenCLInitialized = 1;
    }

    return true;
}

OPENCL_IPP_API OpenclIppStatus Opencl_ippiFilterSobelVertGetBufferSize_32f_C1R(OpenclIppiSize dstRoiSize,
                                                                               OpenclIppiMaskSize mSize,
                                                                               OpenclIpp32s *bSize)
{
    
    /* It is a placeholder currently. */
    return true;
}

OPENCL_IPP_API OpenclIppStatus Opencl_ippiFilterSobelVertBorder_32f_C1R(const OpenclIpp32f* pIn, 
                                                                        OpenclIpp32s srcStep,
                                                                        OpenclIpp32f* pOut,
                                                                        OpenclIpp32s dstStep,
                                                                        OpenclIppiSize dstRoiSize,
                                                                        OpenclIppiMaskSize mSize,
                                                                        OpenclIppiBorderType ippBorderRepl,
                                                                        OpenclIpp32f borderValue,
                                                                        OpenclIpp8u *workbuffer)
{
    /* Only few arguments are used currently. All are there to be compatible with ipp */
    OpenclIppStatus err = 1;

    OpenclIpp32s bitWidth = 32;

    err = OpenCLSobel_Init(&infoDeviceOcl, pIn, pOut, dstRoiSize, mSize, bitWidth);
    if (err != 1)
    {
        return err;
    }

    err = OpenCLSobel_Run(&infoDeviceOcl);
    if (err != 1)
    {
        return err;
    }

    return err;
}

OPENCL_IPP_API void OpenCL_ippSobel_32f_clean()
{
    OpenCLSobelDeInit();
}


OPENCL_IPP_API void OpenCLIppDeInit()
{
    clReleaseCommandQueue(infoDeviceOcl.mQueue);
    clReleaseDevice(infoDeviceOcl.mDevice);
    clReleaseContext(infoDeviceOcl.mCtx);
}