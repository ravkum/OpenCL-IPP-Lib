#ifndef __DE_NOISING_2DFilter_H__
#define __DE_NOISING_2DFilter_H__

#pragma once

/*#ifdef ECHOPROCESSOR_EXPORTS
#define ECHOPROCESSOR_API __declspec(dllexport)
#else
#define ECHOPROCESSOR_API __declspec(dllimport)
#endif*/
#define ECHOPROCESSOR_API 


#define LOG_DATA_TYPE					float
typedef struct ProcParam BProcParam;
struct	ProcParam{
	int kernalSizeDeNoising;
	int numVectors;
	int numSamples;
	int dir;
};


							
class ECHOPROCESSOR_API DeNoising2DFilter
{
public:
	DeNoising2DFilter(void);
	~DeNoising2DFilter(void);

	void Config(BProcParam& bProcParam);
	void DeNoisingProc(LOG_DATA_TYPE* pInOut, BProcParam& bProcParam);
	LOG_DATA_TYPE* myMeanP;
protected:
	void Mean2DFilter(LOG_DATA_TYPE* pInOut, LOG_DATA_TYPE* pMean, BProcParam& bProcParam);
	void DeNoisingFilter(LOG_DATA_TYPE* pInOut, LOG_DATA_TYPE* pMean, BProcParam& bProcParam);
	void SafeDeleteBuffers();
	void MakeMeanFilterCoefficient(float* pMeanFilterCoeff, int filterSize);

	inline bool IsVaildKernelSize(int kernelSize);

protected:
	LOG_DATA_TYPE* myMeanBorderP;	// LOG_DATA_TYPE = Short type
	
	float* myMeanFilterCoeffP;

	int myDeNoisingBufferSize;
	int myOldKernelSize;
};


#endif //__DE_NOISING_2DFilter_H__
