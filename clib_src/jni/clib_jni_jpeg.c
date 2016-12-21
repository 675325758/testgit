#include "clib_jni.h"
#include "../src/h264/yuv2bmp.h"
#include <assert.h>

static u_int16_t jpegDataLen = 0;
static bool isSOFTag = false;
static bool isEndOfFile = false;
static RS _parse_jpeg(u_int8_t *pData, u_int32_t JpgLen, u_int32_t *pOutW, u_int32_t *pOutH);
static RS _find_next_marker(u_int8_t *pSgm, u_int32_t JpgLen, u_int16_t *jumpOffSet);
typedef enum{
    JPEG_MARKER_SOF0 = 0xc0,
    JPEG_MARKER_SOF1 = 0xc1,
    JPEG_MARKER_SOF2 = 0xc2,
    JPEG_MARKER_SOF3 = 0xc3,
    JPEG_MARKER_SOF5 = 0xc5,
    JPEG_MARKER_SOF6 = 0xc6,
    JPEG_MARKER_SOF7 = 0xc7,
    JPEG_MARKER_JPG   = 0xc8,
    JPEG_MARKER_SOF9 = 0xc9,
    JPEG_MARKER_SOF10 = 0xca,
    JPEG_MARKER_SOF11 = 0xcb,
    JPEG_MARKER_SOF13 = 0xcd,
    JPEG_MARKER_SOF14 = 0xce,
    JPEG_MARKER_SOF15 = 0xcf,
    JPEG_MARKER_DHT   = 0xc4,
    JPEG_MARKER_DAC   = 0xcc,
    JPEG_MARKER_RST0 = 0xd0,
    JPEG_MARKER_SOI   = 0xd8,
    JPEG_MARKER_EOI   = 0xd9,
    JPEG_MARKER_SOS   = 0xda,
    JPEG_MARKER_DQT   = 0xdb,
    JPEG_MARKER_DNL   = 0xdc,
    JPEG_MARKER_DRI   = 0xdd,
    JPEG_MARKER_DHP   = 0xde,
    JPEG_MARKER_EXP   = 0xdf,
    JPEG_MARKER_APP0 = 0xe0,
    JPEG_MARKER_APP1 = 0xe1,
    JPEG_MARKER_APP2 = 0xe2,
    JPEG_MARKER_APP3 = 0xe3,
    JPEG_MARKER_APP4 = 0xe4,
    JPEG_MARKER_APP5 = 0xe5,
    JPEG_MARKER_APP6 = 0xe6,
    JPEG_MARKER_APP7 = 0xe7,
    JPEG_MARKER_APP8 = 0xe8,
    JPEG_MARKER_APP9 = 0xe9,
    JPEG_MARKER_APP10 = 0xea,
    JPEG_MARKER_APP11 = 0xeb,
    JPEG_MARKER_APP12 = 0xec,
    JPEG_MARKER_APP13 = 0xed,
    JPEG_MARKER_APP14 = 0xee,
    JPEG_MARKER_APP15 = 0xef,
    JPEG_MARKER_COMMENT = 0xfe,
    JPEG_MARKER_TEM   = 0x01,
    JPEG_MARKER_ERROR = 0x100
}JPEG_MARKER;

static RS parse_jpeg(u_int8_t *pData, u_int32_t JpgLen, u_int32_t *pOutW, u_int32_t *pOutH)
{
    u_int16_t          head_len = 3;//include 0XFF 0XD8 0XFF
    u_int16_t          left_len = 0;
    u_int16_t          marker_offset = 0;
    u_int8_t           *pJpg = pData;
    RS    retVal = RS_ERROR;
    u_int8_t *tempHigh = NULL;
    u_int8_t *tempLow = NULL;
    if(pJpg == NULL)
    {
        LOGE("[%s]Fatal error!!!, line:%d\n",__FUNCTION__, __LINE__);
        return retVal;
    }
    //FF D8 FF E0 00 10 4A 46 49 46 00 01 04 00 48 00 48 00 00 FF DB 00 43 00 05 04 03 ......
    if((*pJpg == 0xFF)&&((*(++pJpg)) == 0xD8)) //SOI Start of Image --JPEG格式文件的判断标志
    {
        pJpg++;                 //point to APP0's 0xFF
        left_len = JpgLen - head_len;
        do
        {
            pJpg++;             //point to APP0's marker   SOI has no length data
            left_len = left_len - 2 - marker_offset;//includes 0xFF 0x**
            
            if((left_len <= 3) && (isSOFTag == false))
            {
                printf("[%s]No more data to search the SOF0 data[Can not find the usefull JPEG data]\n",__FUNCTION__);
                break;
            }
            retVal = _find_next_marker(pJpg, left_len, &marker_offset);
            assert(retVal == RS_OK);
            printf("marker_offset:%d\n",marker_offset);
            if(isSOFTag == false && isEndOfFile == true)//did not find the SOF0 but has reached the end of the file
            {
                LOGE("[%s]Reach the end of the file\n",__FUNCTION__);
                retVal = RS_ERROR;
                break;
            }
            if(isSOFTag == true)//......parseSOF
            {
                pJpg += 4;//跳转到包含高度字节的high字节处
                tempHigh = pJpg;
                printf("tempHigh:0x%2x[should be 0x02]\n",*tempHigh);
                pJpg++;//跳转到包含高度字节的low字节处
                tempLow = pJpg;
                printf("tempLow:0x%2x[should be 0xDB]\n",*tempLow);
                *pOutH = (tempHigh[0]<<8)|tempLow[0];
                printf("Height:%d\n",*pOutH);
                pJpg++;//跳转到包含宽度字节的high字节处
                tempHigh = pJpg;
                printf("tempHigh:0x%2x[should be 0x01]\n",*tempHigh);
                pJpg++;//跳转到包括宽度字节的low字节处
                tempLow = pJpg;
                printf("tempLow:0x%2x[should be 0xDB]\n",*tempLow);
                *pOutW = (tempHigh[0]<<8)|tempLow[0];
                printf("Width:%d\n",*pOutW);
                isSOFTag = false;
                retVal = RS_OK;
                LOGE("[%s]Find SOF and send W/H[%d/%d] to APP\n",__FUNCTION__,*pOutW, *pOutH);
                break;
            }
            pJpg += marker_offset;          //jump the segment len
            pJpg++;                         //0xFF header
        }while(*pJpg == 0xFF); //while（pJpg != NULL）
    }
    else
    {
        LOGE("[%s]The file is not JPEG Data!line:%d\n",__FUNCTION__,__LINE__);
        retVal = RS_ERROR;
    }
    return retVal;
}

RS parse_pic(u_int8_t *pData, u_int32_t JpgLen, u_int32_t *pOutW, u_int32_t *pOutH) {
	if(JpgLen < 2) {
		return RS_ERROR;
	}
	if(pData[0] != 'B' && pData[1] != 'M')  {
		//解析jpeg
		return parse_jpeg(pData, JpgLen, pOutW, pOutH);
	} else {
		//解析BMP
		if(JpgLen < sizeof(bm_t)) {
			return RS_ERROR;
		}
		bm_t* bmp = (bm_t *) pData;
		*pOutW = htoll(bmp->info.biWidth);
		*pOutH = htoll(bmp->info.biHeight);
		return RS_OK;
	}
}
//GLubyte Image[256][256][3];
void get_pic_data(u_int8_t *pData, u_int32_t JpgLen, u_int8_t **data, u_int32_t Width, u_int32_t Heigth) {
	if(JpgLen < 2) {
		*data = NULL;
		return;
	}
	if(pData[0] != 'B' && pData[1] != 'M')  {
		//解析jpeg
		//return parse_jpeg(pData, JpgLen, pOutW, pOutH);
		*data = NULL;
		return;
	} else {
		//解析BMP
		if(JpgLen < sizeof(bm_t)) {
			*data = NULL;
			return;
		}
		
		bm_t* bmp = (bm_t *) pData;
		int i = 0;
		/*for(i = 0 ; i < Width * Heigth * 3 ; i += 3) {
			u_int8_t tmpRGB;
			tmpRGB = bmp->data[i];
			bmp->data[i] = bmp->data[i+2];
			bmp->data[i+2] = tmpRGB;
		}*/
		u_int8_t temp;
		for(i=0; i< bmp->info.biSizeImage; i += bmp->info.biBitCount/8)
		{                                                           // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)  
        	temp = bmp->data[i];                              // Temporarily Store The Value At Image Data 'i'  
        	bmp->data[i] = bmp->data[i + 2];        // Set The 1st Byte To The Value Of The 3rd Byte  
        	bmp->data[i + 2] = temp;                        // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)  
        }
		/*int i=bmp->file.bfOffBits,j=0,k=0;
                
                while(i<bmp->file.bfSize)
                {
                    for(j=0;j<Width;j++)
                        for(k=0;k<Heigth;k++)
                        {
                            memcpy(Image[j][k]+2, data + i, 1);
                            memcpy(Image[j][k]+1, data + i +1, 1);
                            memcpy(Image[j][k], data + i + 2, 1);
                            i=i+3;
                        }  
                }*/
		*data = bmp->data;
		return;
	}
}

static RS _find_next_marker(u_int8_t *pSgm, u_int32_t JpgLen, u_int16_t *jumpOffSet)
{
    u_int16_t       markerLen;
    u_int8_t       *pData = pSgm;
    JPEG_MARKER marker = *pSgm;     //The first byte is segment tag
    u_int8_t *tempHigh = NULL;
    u_int8_t *tempLow = NULL;
    printf("Marker:0x%x......\n",marker);
    switch (marker)
    {
        case JPEG_MARKER_SOI:
            *jumpOffSet = 0;
            break;
        case JPEG_MARKER_EOI:
            *jumpOffSet = 0;
            isEndOfFile = true;
            break;
        case JPEG_MARKER_COMMENT:
        case JPEG_MARKER_DHT:
        case JPEG_MARKER_DQT:
        case JPEG_MARKER_DRI:
        case JPEG_MARKER_SOF2:
        case JPEG_MARKER_SOF1:
        case JPEG_MARKER_SOF3:
        case JPEG_MARKER_SOF5:
        case JPEG_MARKER_SOF6:
        case JPEG_MARKER_SOF7:
        case JPEG_MARKER_SOF9:
        case JPEG_MARKER_SOF10:
        case JPEG_MARKER_SOF11:
        case JPEG_MARKER_SOF13:
        case JPEG_MARKER_SOF14:
        case JPEG_MARKER_SOF15:
        case JPEG_MARKER_SOS:
            
        case JPEG_MARKER_APP0:
        case JPEG_MARKER_APP1:
        case JPEG_MARKER_APP2:
        case JPEG_MARKER_APP3:
        case JPEG_MARKER_APP4:
        case JPEG_MARKER_APP5:
        case JPEG_MARKER_APP6:
        case JPEG_MARKER_APP7:
        case JPEG_MARKER_APP8:
        case JPEG_MARKER_APP9:
        case JPEG_MARKER_APP10:
        case JPEG_MARKER_APP11:
        case JPEG_MARKER_APP12:
        case JPEG_MARKER_APP13:
        case JPEG_MARKER_APP14:
        case JPEG_MARKER_APP15:
        {
            pData++;
            tempHigh = pData;//high len
            /*
             *加上这个判断是由于有些JPEG数据是乱的。
             *如果找到了正在合法的marker，但是紧接着
             *的数据不是长度数据而是另外的marker，则offset=0
             */
            if(*tempHigh == 0xFF)
            {
                printf("This sector has no length Data \n");
                *jumpOffSet = 0;
                break;
            }
            printf("tempHigh:0x%x\n",*tempHigh);
            pData++;
            tempLow = pData;//low len
            printf("tempLow:0x%x\n",*tempLow);
            
            markerLen = (tempHigh[0]<<8)|tempLow[0];
            printf("Marker:0x%x len:%d\n",marker, markerLen);
            *jumpOffSet = markerLen;
            break;
        }
        case JPEG_MARKER_SOF0:
        {
            isSOFTag = true;
            pData++;
            tempHigh = pData;//high len
            if(*tempHigh == 0xFF)
            {
                printf("This sector has no length Data \n");
                *jumpOffSet = 0;
                isSOFTag = false;
                break;
            }
            printf("tempHigh:0x%x\n",*tempHigh);
            pData++;
            tempLow = pData;//low len
            printf("tempLow:0x%x\n",*tempLow);         
            
            markerLen = (tempHigh[0]<<8)|tempLow[0];
            printf("Marker:0x%x len:%d\n",marker, markerLen);
            *jumpOffSet = markerLen;
            break;
        }
        default:
            if(0 == marker)
            {
                *jumpOffSet = 0;
                printf("[%s]Line:%d Is not a marker!\n",__FUNCTION__, __LINE__);
            }
            break;
    }
    return RS_OK;
}