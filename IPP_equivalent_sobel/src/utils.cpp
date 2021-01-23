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
* @file <utils.cpp>
*
* @brief Contains utility functions used by all the samples
*
********************************************************************************
*/
#include "utils.h"
//#define USE_OPENCL_CPU
/**
*******************************************************************************
*  @fn     timerStart
*  @brief  Starts the timer 
*
*  @param[in] mytimer  : pointer to the timer
*
*  @return void 
*******************************************************************************
*/
void timerStart(timer* mytimer)
{
#ifdef _WIN32
    QueryPerformanceCounter(&mytimer->start);
#elif defined __MACH__
    mytimer->start = mach_absolute_time();
#else
    struct timespec s;
    clock_gettime(CLOCK_MONOTONIC, &s);
    mytimer->start = (long long)s.tv_sec * (long long)1.0E6 + (long long)s.tv_nsec / (long long)1.0E3;
#endif
}
/**
*******************************************************************************
*  @fn     timerCurrent
*  @brief  Get current time 
*
*  @param[in] mytimer  : pointer to the timer
*
*  @return double : current time
*******************************************************************************
*/
double timerCurrent(timer* mytimer)
{
#ifdef _WIN32
    LARGE_INTEGER stop, frequency;
    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&frequency);
    double time = ((double)(stop.QuadPart - mytimer->start.QuadPart) / frequency.QuadPart);
#elif defined __MACH__
    static mach_timebase_info_data_t info = { 0, 0 };
    if (info.numer == 0)
        mach_timebase_info(&info);
    long long stop = mach_absolute_time();
    double time = ((stop - mytimer->start) * (double) info.numer / info.denom) / 1.0E9;
#else
    struct timespec s;
    long long stop;
    clock_gettime(CLOCK_MONOTONIC, &s);
    stop = (long long)s.tv_sec * (long long)1.0E6 + (long long)s.tv_nsec / (long long)1.0E3;
    double time = ((double)(stop - mytimer->start) / 1.0E6);
#endif
    return time;
}
