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
* @file <separableFilter.cl>
*
* @brief This file implements the separable filter openCL kernel.
*
********************************************************************************
*/
//#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define T1 float

/**
*******************************************************************************
*  @fn     separableFilter
*  @brief  This is the separable filter. It applies
*           the coloumn vector convolution first and then applies the row vector convolution.
*
*  @param[in] input          : Buffer containing input image 
*  @param[in] row_filter     : Buffer containing the row convoluion kernel
*  @param[in] col_filter     : Buffer containing the col convoluion kernel
*  @param[out] output        : Buffer containing the output of filter
*  @param[in] nWidth         : Image width in pixels
*  @param[in] nHeight        : Image height in pixels
*  @param[in] nExWidth       : Padded image width in pixels
* 
*******************************************************************************
*/
 
__kernel 
__attribute__((reqd_work_group_size(LOCAL_XRES, LOCAL_YRES, 1)))
void separableFilter(
                    __global T1 *input,
                    __global float *row_filter,
                    __global float *col_filter,
					__global float *row_filter1,
                    __global float *col_filter1,					
					__global T1 *output,
                    int nWidth,
                    int nHeight,
                    int nExWidth)                   
{
    __local float local_output[LOCAL_XRES * (LOCAL_YRES + FILTERSIZE - 1)];
	__local float local_output1[LOCAL_XRES * (LOCAL_YRES + FILTERSIZE - 1)];

    int col = get_global_id(0);
    int row = get_global_id(1);
    int lid_x = get_local_id(0);
    int lid_y = get_local_id(1);

    int cnt = 0;
	float sum = 0.0f;
	float sum1 = 0.0f;
	int start_col, start_row;    

#if USE_LDS == 1
	__local T1 local_input[(LOCAL_XRES + FILTERSIZE - 1) * (LOCAL_YRES + FILTERSIZE - 1)];
		int tile_xres = (LOCAL_XRES + FILTERSIZE - 1);
		int tile_yres = (LOCAL_YRES + FILTERSIZE - 1);
		/***************************************************************************************
		* If using LDS, get the data to local memory. Else, get the global memory indices ready 
		***************************************************************************************/
		
    
		start_col = get_group_id(0) * LOCAL_XRES;
		start_row = get_group_id(1) * LOCAL_YRES; 
		start_col -= (FILTERSIZE/2); 
		start_row -= (FILTERSIZE/2);		
 
		int lid = lid_y * LOCAL_XRES + lid_x; 
		int gx, gy;
		int lx,ly;
    
		 /*********************************************************************
		 * Read input from global buffer and put in local buffer 
		 * Read 256 global memory locations at a time (256 WI). 
		 * Conitnue in a loop till all pixels in the tile are read.
		 **********************************************************************/

		do
		{
			gy = lid / tile_xres;
			gx = lid - gy * tile_xres; 

			ly=start_row + gy;
			lx=start_col + gx;
			lx=min(max(0,lx),nWidth-1);
			ly=min(max(0,ly),nHeight-1);
			local_input[lid] = input[ ly* nExWidth + lx];
			lid += (LOCAL_XRES * LOCAL_YRES);
		} while (lid < (tile_xres * tile_yres));

	barrier(CLK_LOCAL_MEM_FENCE);
	start_col = lid_x;
#pragma unroll FILTERSIZE
	 for (int i = start_col; i < start_col + FILTERSIZE; i++) {
	    sum = mad(convert_float(local_input[lid_y * tile_xres + i]), col_filter[cnt],(float) sum);    
		sum1 = mad(convert_float(local_input[lid_y * tile_xres + i]), col_filter1[cnt++],(float) sum1);    
	}    
	/***********************************************************************************
	* Output is stored in local memory
	************************************************************************************/
	local_output[lid_y * LOCAL_XRES + lid_x] = sum;
	local_output1[lid_y * LOCAL_XRES + lid_x] = sum1;

	/***************************************************************************************
	* Row-wise convolution of pixels in the remaining rows
	***************************************************************************************/
	if (lid_y < FILTERSIZE - 1) 
	{
		cnt = 0;
		sum = 0.0f;
		sum1 = 0.0f;

#pragma unroll FILTERSIZE
		for (int i = start_col; i < start_col + FILTERSIZE; i++) {
	           sum = mad(convert_float(local_input[(lid_y + LOCAL_YRES) * tile_xres + i]), col_filter[cnt], (float)sum);  
			   sum1 = mad(convert_float(local_input[(lid_y + LOCAL_YRES) * tile_xres + i]), col_filter1[cnt++], (float)sum1);  
		}
		/***********************************************************************************
		* Again the output is stored in local memory
		************************************************************************************/
		local_output[(lid_y + LOCAL_YRES) * LOCAL_YRES + lid_x] = sum;
		local_output1[(lid_y + LOCAL_YRES) * LOCAL_YRES + lid_x] = sum1;
	}
#else   
		/************************************************************************
		* Non - LDS implementation
		* Read pixels directly from global memory
		************************************************************************/
		start_col = col-(FILTERSIZE/2); 
		start_row = row	-(FILTERSIZE/2);
		/***********************************************************************************
		* Row-wise convolution - Inputs will be read from local or global memory         
		************************************************************************************/	
		sum=0.0f;
		cnt = 0;
		int idx;
#pragma unroll FILTERSIZE
	    for (int i = start_col; i < start_col + FILTERSIZE; i++) {
			idx=min(max(0,i),nWidth-1);
			int cRow=min(max(0,start_row),nHeight-1);
			int index=cRow * nExWidth + idx;		
			sum = mad(convert_float(input[index]), col_filter[cnt++], sum);                 
		}
    
		/***********************************************************************************
		* Output is stored in local memory
		************************************************************************************/
		local_output[lid_y * LOCAL_XRES + lid_x] = sum;

		/***************************************************************************************
		* Row-wise convolution of pixels in the remaining rows
		***************************************************************************************/
		if (lid_y < FILTERSIZE - 1) 
		{
			cnt = 0;
			sum = 0.0f;

#pragma unroll FILTERSIZE
			for (int i = start_col; i < start_col + FILTERSIZE; i++) {
				idx=min(max(0,i),nWidth-1);
				int cRow=min(start_row+LOCAL_YRES,nHeight-1);
				int index=cRow * nExWidth + idx;		
				sum = mad(input[index], col_filter[cnt++], sum);                    
			}
			/***********************************************************************************
			* Again the output is stored in local memory
			************************************************************************************/
			local_output[(lid_y + LOCAL_YRES) * LOCAL_YRES + lid_x] = sum;
		}


#endif
    /***********************************************************************************
    * Wait for all the local WIs to finish row-wise convolution.
    ************************************************************************************/
    barrier(CLK_LOCAL_MEM_FENCE); 

   /************************************************************************************
    * Column-wise convolution - Input is the output of row-wise convolution
    * Inputs are always read from local memory. 
    * The output is written to global memory.
    ***********************************************************************************/
	    start_row = lid_y;
		sum = 0.0f;
		sum1 = 0.0f;
		cnt = 0;

#pragma unroll FILTERSIZE
		for (int i = start_row; i < start_row + FILTERSIZE; i++) {	
			sum = mad(convert_float(local_output[i * LOCAL_XRES + lid_x]), row_filter[cnt], (float)sum);        
			sum1 = mad(convert_float(local_output1[i * LOCAL_XRES + lid_x]), row_filter1[cnt++],(float) sum1);        
		}
    if(row<nHeight && col<nWidth)
		/* Save Output */
		//output[row * nWidth + col] = ROUND(sum);   	
		//output[row * nWidth + col] = -sum;   	
		output[row * nWidth + col] = fabs(sum)+fabs(sum1);   	
	
}
