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

#ifndef _OPENCL_IPP_DEFS_H_
#define _OPENCL_IPP_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char   OpenclIpp8u;
typedef unsigned short  OpenclIpp16u;
typedef unsigned int    OpenclIpp32u;

typedef signed char     OpenclIpp8s;
typedef signed short    OpenclIpp16s;
typedef signed int      OpenclIpp32s;
typedef float           OpenclIpp32f;
typedef double          OpenclIpp64f;

typedef OpenclIpp32u    OpenclIppiMaskSize;

typedef OpenclIpp32s    OpenclIppStatus;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} OpenclIppRect;

typedef struct {
    int x;
    int y;
} OpenclIppPoint;

typedef struct {
    int width;
    int height;
} OpenclIppiSize;

typedef enum _OpenclIppiBorderType {
    OpenclippBorderConst     =  0,
    OpenclippBorderRepl      =  1,
    OpenclippBorderWrap      =  2,
    OpenclippBorderMirror    =  3, /* left border: 012... -> 21012... */
    OpenclippBorderMirrorR   =  4, /* left border: 012... -> 210012... */
    OpenclippBorderInMem     =  6,
    OpenclippBorderTransp    =  7,
    OpenclippBorderInMemTop     =  0x0010,
    OpenclippBorderInMemBottom  =  0x0020,
    OpenclippBorderInMemLeft    =  0x0040,
    OpenclippBorderInMemRight   =  0x0080
} OpenclIppiBorderType;

#ifdef __cplusplus
}
#endif


#endif