#include "DeNoising2DFilter.h"
#include <math.h>
#include<stdio.h>
#include <memory.h>
#include "OpenCLIpp.h"
#include "ippcore.h"
#include "ipps.h"
#include "ippi.h"

#include<string.h>
#define 	SAFE_DELETE_ARRAY(a)   { delete [] (a); (a) = NULL; }
#define max(x, y) ((x > y)?x:y)


//#define LOG_DATA_TYPE				short
//#define SAFE_DELETE_ARRAY(p)		{ if(p)		{ delete[] (p);   (p)=NULL; } }

DeNoising2DFilter::DeNoising2DFilter(void)
	: myMeanBorderP(NULL)
	, myMeanP(NULL)
	, myMeanFilterCoeffP(NULL)

	, myDeNoisingBufferSize(-1)
	, myOldKernelSize(-1)
{
}

DeNoising2DFilter::~DeNoising2DFilter(void)
{
	SAFE_DELETE_ARRAY(myMeanFilterCoeffP);

	SafeDeleteBuffers();
}

void DeNoising2DFilter::SafeDeleteBuffers()
{
	SAFE_DELETE_ARRAY(myMeanBorderP);
	SAFE_DELETE_ARRAY(myMeanP);
}

void DeNoising2DFilter::Config(BProcParam& bProcParam)
{
	int kernelSize = bProcParam.kernalSizeDeNoising;	// KernelSize: 3 (3X3), 5 (5X5), 7 (7X7)
	if (!IsVaildKernelSize(kernelSize))
	{
		return;
	}

	int numVectors = bProcParam.numVectors;		// Height
	int numSamples = bProcParam.numSamples;		// Width
	int requiredBufferLength = numVectors * numSamples;

	bool isBufferSizeChanged = myDeNoisingBufferSize != requiredBufferLength;
	bool isKernelSizeChanged = myOldKernelSize != kernelSize;


	if (isBufferSizeChanged || isKernelSizeChanged)
	{
		if (requiredBufferLength > 0)
		{
			int kernelSize_1 = kernelSize - 1;
			int nDataSizeBorder = (numVectors + kernelSize_1) * (numSamples + kernelSize_1);

			SafeDeleteBuffers();

			myMeanP = new LOG_DATA_TYPE[requiredBufferLength];
			myMeanBorderP = new LOG_DATA_TYPE[nDataSizeBorder];

			memset(myMeanP, 0, requiredBufferLength * sizeof(LOG_DATA_TYPE));
			memset(myMeanBorderP, 0, nDataSizeBorder * sizeof(LOG_DATA_TYPE));
		}

		if (isKernelSizeChanged && requiredBufferLength > 0)
		{
			int meanFilterSize = kernelSize * kernelSize;
			SAFE_DELETE_ARRAY(myMeanFilterCoeffP);
			myMeanFilterCoeffP = new float[meanFilterSize];

			// Make Mean Filter Coefficients : KernelSize가 변경된 경우에만 수행
			MakeMeanFilterCoefficient(myMeanFilterCoeffP, meanFilterSize);
		}

		myDeNoisingBufferSize = requiredBufferLength;
		myOldKernelSize = kernelSize;
	}
}

void DeNoising2DFilter::MakeMeanFilterCoefficient(float* pMeanFilterCoeff, int filterSize)
{
	// Mean Filter Coefficient 값을 생성
	int meanFilterSize = max(1, filterSize);
	float divValue = 1.f / (float)meanFilterSize; 
	
	for (int i = 0; i < meanFilterSize; i++)
	{
		pMeanFilterCoeff[i] = divValue;
	}
	
}

void DeNoising2DFilter::DeNoisingProc(LOG_DATA_TYPE* pInOut, BProcParam& bProcParam)
{
	int kernelSize = bProcParam.kernalSizeDeNoising;	// KernelSize: 3 (3X3), 5 (5X5), 7 (7X7)
	if (!IsVaildKernelSize(kernelSize) || myMeanBorderP == NULL || myMeanP == NULL || myMeanFilterCoeffP == NULL)
	{
		return;
	}

	Mean2DFilter(pInOut, myMeanP, bProcParam);
}

void DeNoising2DFilter::Mean2DFilter(LOG_DATA_TYPE* pInOut, LOG_DATA_TYPE* pMean, BProcParam& bProcParam)
{
	int numVectors = bProcParam.numVectors;		// Height
	int numSamples = bProcParam.numSamples;		// Widht
	int nDataSize = numVectors * numSamples;
	int kernelSize = bProcParam.kernalSizeDeNoising;	// KernelSize: 3 (3X3), 5 (5X5), 7 (7X7)
	int kernelSize_1 = kernelSize - 1;
	int kernelSize_half = kernelSize >> 1;

    int srcStep = numSamples * sizeof(LOG_DATA_TYPE);	
	int dstStep = srcStep;
    
    int borderHeight = kernelSize_half;
	int borderWidth = kernelSize_half;

#ifdef IPP_PATH	
    IppStatus st = ippStsNoErr;
	IppiSize srcRoiSize = {numSamples, numVectors};
	IppiSize copyRoiSize = {numSamples + kernelSize_1, numVectors + kernelSize_1};
	IppiSize dstRoiSize = {numSamples, numVectors};
    const Ipp32f* pIn  = (const Ipp32f*)pInOut;	
	Ipp32f* pOut = (Ipp32f*)pMean;
	int bSize;
	IppiMaskSize mSize;
	if(kernelSize==3)
		mSize=ippMskSize3x3;
	else
		mSize=ippMskSize5x5;
	Ipp32f *workbuffer = NULL;

#else
    OpenclIppStatus st;
    OpenclIppiSize srcRoiSize = {numSamples, numVectors};
	OpenclIppiSize copyRoiSize = {numSamples + kernelSize_1, numVectors + kernelSize_1};
	OpenclIppiSize dstRoiSize = {numSamples, numVectors};
    const OpenclIpp32f* pIn  = (const OpenclIpp32f*)pInOut;	
	OpenclIpp32f* pOut = (OpenclIpp32f*)pMean;
	int bSize;

	OpenclIppiMaskSize mSize;
	
    if(kernelSize==3)
		mSize=3;
	else
		mSize=5;
    
    OpenclIpp32f *workbuffer = NULL;
#endif

#ifdef IPP_PATH
	st = ippiFilterSobelVertGetBufferSize_32f_C1R(dstRoiSize,mSize,&bSize);	
#else
    st = Opencl_ippiFilterSobelVertGetBufferSize_32f_C1R(dstRoiSize,mSize,&bSize);
#endif

#ifdef IPP_PATH
	workbuffer=ippsMalloc_32f(bSize);	
	st = ippiFilterSobelVertBorder_32f_C1R(pIn,srcStep,pOut,dstStep,dstRoiSize,mSize,ippBorderRepl,0,(Ipp8u *)workbuffer);
#else
    st = Opencl_ippiFilterSobelVertBorder_32f_C1R(pIn,srcStep,pOut,dstStep,dstRoiSize,mSize,OpenclippBorderRepl,0,(Ipp8u *)workbuffer);
    
#endif

#ifdef IPP_PATH
    /* 
    * The horizontal sobel is comined with vertical sobel in openclIpp. 
    * So, OpenclIpp path for this part of the code is not needed.
    */

	st=ippiFilterSobelHorizGetBufferSize_32f_C1R(dstRoiSize,mSize,&bSize);	

	//workbuffer=ippsMalloc_32f(bSize);	

	int requiredBufferLength = numVectors * numSamples;
	Ipp32f* pOut1=new LOG_DATA_TYPE[requiredBufferLength];
	st = ippiFilterSobelHorizBorder_32f_C1R(pIn,srcStep,pOut1,dstStep,dstRoiSize,mSize,ippBorderRepl,0,(Ipp8u *)workbuffer);

  	st= ippiAbs_32f_C1R(pOut, srcStep, pOut, dstStep, dstRoiSize);
	st= ippiAbs_32f_C1R(pOut1, srcStep, pOut1, dstStep, dstRoiSize);
	st= ippiAdd_32f_C1R(pOut, srcStep, pOut1, srcStep,  pOut,  dstStep, dstRoiSize);
    ippsFree(workbuffer);
#endif

}

bool DeNoising2DFilter::IsVaildKernelSize(int kernelSize)
{
	bool bRet = false;

	// KernelSize : 3 x 3, 5 X 5
	if (kernelSize == 3 || kernelSize == 5)
	{
		bRet = true;
	}

	return bRet;
}