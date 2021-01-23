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

#ifndef _OPENCL_IPP_H_
#define _OPENCL_IPP_H_

#include "OpenclIppDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#   ifdef OPENCL_IPP_API_EXPORTS
#       define OPENCL_IPP_API __declspec(dllexport)
#   else
#       define OPENCL_IPP_API __declspec(dllimport)
#   endif
#else
#   define OPENCL_IPP_API
#endif


typedef struct OpenclSobelSpec OpenclSobelSpec;

OPENCL_IPP_API OpenclIppStatus OpenCLIppInit();

OPENCL_IPP_API OpenclIppStatus Opencl_ippiFilterSobelVertGetBufferSize_32f_C1R(OpenclIppiSize dstRoiSize,
                                                                               OpenclIppiMaskSize mSize,
                                                                               OpenclIpp32s *bSize);

OPENCL_IPP_API OpenclIppStatus Opencl_ippiFilterSobelVertBorder_32f_C1R(const OpenclIpp32f* pIn, 
                                                                        OpenclIpp32s srcStep,
                                                                        OpenclIpp32f* pOut,
                                                                        OpenclIpp32s dstStep,
                                                                        OpenclIppiSize dstRoiSize,
                                                                        OpenclIppiMaskSize mSize,
                                                                        OpenclIppiBorderType ippBorderRepl,
                                                                        OpenclIpp32f borderValue,
                                                                        OpenclIpp8u *workbuffer);

OPENCL_IPP_API void OpenCL_ippSobel_32f_clean();

OPENCL_IPP_API void OpenCLIppDeInit();

/* Globals - Should not be part of th efinal product. Only for poc */


#ifdef __cplusplus
}
#endif

#endif /*_OPENCL_IPP_H_ */
