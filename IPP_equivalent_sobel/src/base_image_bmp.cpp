/*******************************************************************************
** Copyright(C) 2005-2014 Intel Corporation. All Rights Reserved.
**                                                                             
** The source code, information and material ("Material") contained herein is 
** owned by Intel Corporation or its suppliers or licensors, and title 
** to such Material remains with Intel Corporation or its suppliers or 
** licensors. The Material contains proprietary information of Intel or 
** its suppliers and licensors. The Material is protected by worldwide 
** copyright laws and treaty provisions. No part of the Material may be used, 
** copied, reproduced, modified, published, uploaded, posted, transmitted, 
** distributed or disclosed in any way without Intel's prior express written 
** permission. No license under any patent, copyright or other intellectual 
** property rights in the Material is granted to or conferred upon you, 
** either expressly, by implication, inducement, estoppel or otherwise. 
** Any license under such intellectual property rights must be express and 
** approved by Intel in writing. Unless otherwise agreed by Intel in writing, 
** you may not remove or alter this notice or any other notice embedded in 
** Materials by Intel or Intel's suppliers or licensors in any way.
*/

#include "base_image.h"
#include <memory.h>
struct BMPHeader
{
    // file header
    unsigned short  bfType;
    unsigned int    bfSize;
    unsigned short  bfReserved1;
    unsigned short  bfReserved2;
    unsigned int    bfOffBits;

    // image header
    unsigned int    biSize;
    unsigned int    biWidth;
    int             biHeight;
    unsigned short  biPlanes;
    unsigned short  biBitCount;
    unsigned int    biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYPelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
};

struct RGBquad
{
    unsigned char    rgbBlue;
    unsigned char    rgbGreen;
    unsigned char    rgbRed;
    unsigned char    rgbReserved;
};

int BmpReadData(FILE *pFile, Image *pData)
{
    if(!pData || !pFile)
        return -1;

    RGBquad      palette[256];
    BMPHeader    header;

    memset(&palette[0], 0, sizeof(palette));
    memset(&header, 0, sizeof(BMPHeader));

    // read header
    fread(&header.bfType, 1, sizeof(header.bfType), pFile);
    fread(&header.bfSize, 1, sizeof(header.bfSize), pFile);

    if(header.bfType != 'MB')
        return -1;

    fread(&header.bfReserved1, 1, sizeof(header.bfReserved1), pFile);
    fread(&header.bfReserved2, 1, sizeof(header.bfReserved2), pFile);
    fread(&header.bfOffBits, 1, sizeof(header.bfOffBits), pFile);

    fread(&header.biSize, 1, sizeof(header.biSize), pFile);
    fread(&header.biWidth, 1, sizeof(header.biWidth), pFile);
    fread(&header.biHeight, 1, sizeof(header.biHeight), pFile);
    fread(&header.biPlanes, 1, sizeof(header.biPlanes), pFile);
    fread(&header.biBitCount, 1, sizeof(header.biBitCount), pFile);

    if(header.biBitCount != 8 && header.biBitCount != 24 && header.biBitCount != 32)
        return -1;

    fread(&header.biCompression, 1, sizeof(header.biCompression), pFile);

    switch(header.biCompression)
    {
    case 0L: //0L == BI_RGB
        break;

    case 3L: //3L == BI_BITFIELDS (we support only 8uC4 images)
        {
        if(header.biBitCount != 32)
            return -1;
        }
        break;

    default:
        return -1;
    }

    fread(&header.biSizeImage, 1, sizeof(header.biSizeImage), pFile);
    fread(&header.biXPelsPerMeter, 1, sizeof(header.biXPelsPerMeter), pFile);
    fread(&header.biYPelsPerMeter, 1, sizeof(header.biYPelsPerMeter), pFile);
    fread(&header.biClrUsed, 1, sizeof(header.biClrUsed), pFile);
    fread(&header.biClrImportant, 1, sizeof(header.biClrImportant), pFile);

    if(header.biBitCount == 8)
        fread(&palette, 1, sizeof(RGBquad)*256, pFile);
    else if(header.biBitCount == 32 && header.biCompression == 3L)
        fread(&palette, 1, sizeof(RGBquad)*3, pFile);

    pData->m_iWidth   = header.biWidth;
    pData->m_iHeight  = ABS(header.biHeight);
    pData->m_iSamples = header.biBitCount >> 3;

    if(pData->m_iSamples == 1)
        pData->m_color = IC_GRAY;
    else if(pData->m_iSamples == 3)
        pData->m_color = IC_BGR;
    else
        return -1;

    // read data
    if(!header.bfOffBits)
        return -1;

    if(fseek(pFile, header.bfOffBits, SEEK_SET))
        return -1;

    pData->Alloc();

    unsigned int iFileStep = alignValue<unsigned int>(header.biWidth*(header.biBitCount >> 3), 4);

    if(0 < header.biHeight) // read bottom-up BMP
    {
        unsigned char *pPtr = pData->m_pPointer + pData->m_iStep * (pData->m_iHeight - 1);

        for(unsigned int i = 0; i < pData->m_iHeight; i++)
        {
            if(iFileStep != fread((unsigned char*)(pPtr - i * pData->m_iStep), 1, iFileStep, pFile))
                return -1;
        }
    }
    else // read up-bottom BMP
    {
        for(unsigned int i = 0; i < pData->m_iHeight; i++)
        {
            if(iFileStep != fread(pData->m_pPointer + i * pData->m_iStep, 1, iFileStep, pFile))
                return -1;
        }
    }

    if(3L == header.biCompression && header.biBitCount == 32)
    {
//        IppiSize size = {header.biWidth, iHeight};
//        int order[4]  = {3,2,1,0}; // convert from ABGR to RGBA

//        ippiSwapChannels_8u_C4IR(pData->m_pBuffer, iStep, size, order);
    }

    return 0;
}

int BmpWriteData(Image *pData, FILE *pFile)
{
    unsigned int iFileSize;
    unsigned int iImageSize;
    unsigned int iIHSize = 40;
    unsigned int iFHSize = 14;
    unsigned int iFileStep;
    unsigned int i;

    RGBquad    palette[256] = {0};
    BMPHeader  header;

    if(!pData || !pFile)
        return -1;

    iFileStep   = alignValue<unsigned int>(pData->m_iWidth * pData->m_iSamples, 4);
    iImageSize  = iFileStep * pData->m_iHeight;
    iFileSize   = iImageSize + iIHSize + iFHSize;

    header.bfType      = 'MB';
    header.bfSize      = iFileSize;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits   = iIHSize + iFHSize;

    if(pData->m_iSamples == 1)
        header.bfOffBits += sizeof(palette);

    // write header
    fwrite(&header.bfType, 1, sizeof(header.bfType), pFile);
    fwrite(&header.bfSize, 1, sizeof(header.bfSize), pFile);
    fwrite(&header.bfReserved1, 1, sizeof(header.bfReserved1), pFile);
    fwrite(&header.bfReserved2, 1, sizeof(header.bfReserved2), pFile);
    fwrite(&header.bfOffBits, 1, sizeof(header.bfOffBits), pFile);

    header.biSize          = iIHSize;
    header.biWidth         = pData->m_iWidth;
    header.biHeight        = pData->m_iHeight;
    header.biPlanes        = 1;
    header.biBitCount      = (short)(pData->m_iSamples << 3);
    header.biCompression   = 0L;
    header.biSizeImage     = iImageSize;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed       = ((pData->m_iSamples == 1) ? 256 : 0);
    header.biClrImportant  = ((pData->m_iSamples == 1) ? 256 : 0);

    fwrite(&header.biSize, 1, sizeof(header.biSize), pFile);
    fwrite(&header.biWidth, 1, sizeof(header.biWidth), pFile);
    fwrite(&header.biHeight, 1, sizeof(header.biHeight), pFile);
    fwrite(&header.biPlanes, 1, sizeof(header.biPlanes), pFile);
    fwrite(&header.biBitCount, 1, sizeof(header.biBitCount), pFile);
    fwrite(&header.biCompression, 1, sizeof(header.biCompression), pFile);
    fwrite(&header.biSizeImage, 1, sizeof(header.biSizeImage), pFile);
    fwrite(&header.biXPelsPerMeter, 1, sizeof(header.biXPelsPerMeter), pFile);
    fwrite(&header.biYPelsPerMeter, 1, sizeof(header.biYPelsPerMeter), pFile);
    fwrite(&header.biClrUsed, 1, sizeof(header.biClrUsed), pFile);
    fwrite(&header.biClrImportant, 1, sizeof(header.biClrImportant), pFile);

    if(pData->m_iSamples == 1)
    {
        for(i = 0; i < 256; i++)
        {
            palette[i].rgbBlue     = (unsigned char)i;
            palette[i].rgbGreen    = (unsigned char)i;
            palette[i].rgbRed      = (unsigned char)i;
            palette[i].rgbReserved = (unsigned char)0;
        }

        fwrite(&palette[0], 1, sizeof(palette), pFile);
    }

    // write data
    unsigned char *pPtr = (unsigned char*)pData->m_pPointer + pData->m_iStep * (pData->m_iHeight - 1);

    for(i = 0; i < pData->m_iHeight; i++)
    {
        if(iFileStep != fwrite(pPtr - i*pData->m_iStep, 1, iFileStep, pFile))
            return -1;
    }

    return 0;
}
