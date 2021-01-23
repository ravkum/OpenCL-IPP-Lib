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
* @file <TestCode.cpp>
*
* @Conatins the code to compare Intel's IPP and AMD's OpenCL for Denoise filter
*
********************************************************************************
*/
#include "DeNoising2DFilter.h"
#include "base_image.h"
#include "utils.h"
#include "OpenCLIpp.h"


//#define MATCH_IPP_OCL
#define PERFORMANCE_TEST_APP
#define LOOP_COUNT 100
void sobel_ocl(Image *srcData, LOG_DATA_TYPE*in, LOG_DATA_TYPE * out,unsigned char *out8, unsigned int loopCnt);
void sobel_ipp(Image *srcData, DeNoising2DFilter *denoise, LOG_DATA_TYPE*in, LOG_DATA_TYPE * out, unsigned char *out8, unsigned int loopCnt);
bool compareIppOcl(LOG_DATA_TYPE*in, DeNoising2DFilter *denoise, unsigned int width, unsigned height);
void get_32bitData(LOG_DATA_TYPE *out, unsigned char *in, int size)
{
    for (int i = 0; i < size; i++)
    {
        out[i] = in[i];
    }
}
void get_8bitData(LOG_DATA_TYPE *in, unsigned char *out, int size)
{	
    for (int i = 0; i < size; i++)
    {
        out[i] = in[i];
    }
}
void dump_raw_buffer(LOG_DATA_TYPE *buf,int count,char *fname)
{
    FILE *t=fopen(fname,"w");
    for(int i=0;i<count;i++)
        fprintf(t,"%5.3f\n",buf[i]);
    fclose(t);
}

int main(int argc, char *argv[])
{
#ifdef PERFORMANCE_TEST_APP
	int loopCnt=LOOP_COUNT;
#endif 
	if(argc!=2){
		printf("\n e.g. Usage: Denoise.exe lena.bmp ");		
		exit(1);
	}
	char *inFile=argv[1];
	LOG_DATA_TYPE *in,*out;	
    Image srcData;
	srcData.Read(inFile);
	printf("Input info: %dx%d %s\n", srcData.m_iWidth, srcData.m_iHeight, colorName[srcData.m_color]);
	in = (LOG_DATA_TYPE *)malloc(srcData.m_iWidth*srcData.m_iHeight*sizeof(LOG_DATA_TYPE));
	out = (LOG_DATA_TYPE *)malloc(srcData.m_iWidth*srcData.m_iHeight*sizeof(LOG_DATA_TYPE));
	unsigned char *out8 = (unsigned char *)malloc(srcData.m_iWidth*srcData.m_iHeight);
	memset(out8,0,srcData.m_iWidth*srcData.m_iHeight);
	get_32bitData(in,srcData.m_pBuffer,srcData.m_iWidth*srcData.m_iHeight);

    DeNoising2DFilter denoise = DeNoising2DFilter();
	sobel_ipp(&srcData, &denoise, in, out, out8,loopCnt);

}
void sobel_ipp(Image *srcData, DeNoising2DFilter *denoise ,LOG_DATA_TYPE*in, LOG_DATA_TYPE * out,unsigned char *out8, unsigned int loopCnt)
{

    char *OutFile = "output.bmp";
    BProcParam b;
    b.kernalSizeDeNoising = 5;
    b.numSamples = srcData->m_iWidth;
    b.numVectors = srcData->m_iHeight;

    denoise->Config(b);
    denoise->DeNoisingProc(in, b);
#ifdef PERFORMANCE_TEST_APP
    double time_ipp;
    timer t_timer_ipp;
    timerStart(&t_timer_ipp);

    for (int i = 0; i < loopCnt; i++)
    {
        denoise->DeNoisingProc(in, b);
    }
    time_ipp = timerCurrent(&t_timer_ipp);
    time_ipp = 1000 * (time_ipp / loopCnt);
    printf("Average time taken per iteration is %f msec\n", time_ipp);
#endif

    //dump_raw_buffer(denoise->myMeanP, srcData->m_iWidth*srcData->m_iHeight, "ipp.dat");
    get_8bitData(denoise->myMeanP, out8, srcData->m_iWidth*srcData->m_iHeight);
    Image dstData;
    dstData = *srcData;
    dstData.m_iWidth = srcData->m_iWidth;
    dstData.m_iHeight = srcData->m_iHeight;
    memcpy(dstData.m_pBuffer, out8, srcData->m_iWidth*srcData->m_iHeight);
    printf("Output info: %dx%d %s\n", dstData.m_iWidth, dstData.m_iHeight, colorName[dstData.m_color]);
    dstData.Write(OutFile);

#ifndef IPP_PATH
    OpenCL_ippSobel_32f_clean();
#endif
}


bool compareIppOcl(LOG_DATA_TYPE *ocl, DeNoising2DFilter *denoise,
                   unsigned int width,unsigned height)
{
    for (int i = 0; i<width*height; i++)
    {
        if (denoise->myMeanP[i] != ocl[i])
        {
            printf("\ndifference found IPP Value=%5.3f OCL Value=%5.3f ", denoise->myMeanP[i], ocl[i]);
            return false;
        }
    }
    return true;
}
