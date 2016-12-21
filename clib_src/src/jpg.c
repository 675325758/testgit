#include "client_lib.h"
#include "cl_priv.h"


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

typedef struct {
	bool isSOFTag;
	bool isEndOfFile;
	int jumpOffSet;
} parse_jpg_t;

static RS find_next_marker(parse_jpg_t *pj, u_int8_t *date)
{
    u_int16_t	markerLen;
     JPEG_MARKER marker = *date;     //The first byte is segment tag
    u_int8_t *tempHigh = NULL;
    u_int8_t *tempLow = NULL;
	
    	
    switch (marker) {
    case JPEG_MARKER_SOI:
        pj->jumpOffSet = 0;
        break;
    case JPEG_MARKER_EOI:
        pj->jumpOffSet = 0;
        pj->isEndOfFile = true;
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
        date++;
        tempHigh = date;//high len
        /*
         *加上这个判断是由于有些JPEG数据是乱的。
         *如果找到了正在合法的marker，但是紧接着
         *的数据不是长度数据而是另外的marker，则offset=0
         */
        if(*tempHigh == 0xFF)
        {
            //printf("This sector has no length Data \n");
            pj->jumpOffSet = 0;
            break;
        }
        date++;
        tempLow = date;//low len
        
        markerLen = (tempHigh[0]<<8)|tempLow[0];
        pj->jumpOffSet = markerLen;
        break;
    }
    case JPEG_MARKER_SOF0:
    {
        pj->isSOFTag = true;
        date++;
        tempHigh = date;//high len
        if(*tempHigh == 0xFF)
        {
            pj->jumpOffSet = 0;
            pj->isSOFTag = false;
            break;
        }
        date++;
        tempLow = date;//low len
        
        markerLen = (tempHigh[0]<<8)|tempLow[0];
        pj->jumpOffSet = markerLen;
        break;
    }
    default:
        if(0 == marker) {
            pj->jumpOffSet = 0;
        }
        break;
    }
    return RS_OK;
}

RS parse_jpg_wh(u_int8_t *date, int JpgLen, int *pOutW, int *pOutH)
{
    u_int16_t	head_len = 3;//include 0XFF 0XD8 0XFF
    int	left_len = 0;
    u_int8_t	*pJpg = date;
    RS    retVal = RS_ERROR;
    u_int8_t *tempHigh = NULL;
    u_int8_t *tempLow = NULL;
	parse_jpg_t pj;

    if (pJpg == NULL) {
        log_err(false, "[%s]Fatal error!!!, line:%d\n",__FUNCTION__, __LINE__);
        return retVal;
    }

	memset(&pj, 0, sizeof(pj));
	pj.isSOFTag = false;
	pj.isEndOfFile = false;
	
    //FF D8 FF E0 00 10 4A 46 49 46 00 01 04 00 48 00 48 00 00 FF DB 00 43 00 05 04 03 ......
    if((*pJpg == 0xFF)&&((*(++pJpg)) == 0xD8)) //SOI Start of Image --JPEG格式文件的判断标志
    {
        pJpg++;                 //point to APP0's 0xFF
        left_len = JpgLen - head_len;
        do
        {
            pJpg++;             //point to APP0's marker   SOI has no length data
            left_len = left_len - 2 - pj.jumpOffSet;//includes 0xFF 0x**
            
            if ((left_len <= 3) && (pj.isSOFTag == false)) {
                log_err(false, "[%s]No more data to search the SOF0 data[Can not find the usefull JPEG data]\n",__FUNCTION__);
                break;
            }
            if (find_next_marker(&pj, pJpg) != RS_OK)
				break;
			
            if (pj.isSOFTag == false && pj.isEndOfFile == true)//did not find the SOF0 but has reached the end of the file
            {
                log_err(false, "[%s]Reach the end of the file\n",__FUNCTION__);
                break;
            }
            if(pj.isSOFTag == true)//......parseSOF
            {
                pJpg += 4;//跳转到包含高度字节的high字节处
                tempHigh = pJpg;
                pJpg++;//跳转到包含高度字节的low字节处
                tempLow = pJpg;
                *pOutH = (tempHigh[0]<<8)|tempLow[0];
                pJpg++;//跳转到包含宽度字节的high字节处
                tempHigh = pJpg;
                pJpg++;//跳转到包括宽度字节的low字节处
                tempLow = pJpg;
                *pOutW = (tempHigh[0]<<8)|tempLow[0];
                pj.isSOFTag = false;
				
                retVal = RS_OK;
                log_debug("[%s]Find SOF and send W/H[%d/%d] to APP, parse %u bytes\n",__FUNCTION__,*pOutW, *pOutH, (int)(pJpg-date));
                break;
            }
            pJpg += pj.jumpOffSet;          //jump the segment len
            pJpg++;                         //0xFF header
        }while(*pJpg == 0xFF); //while（pJpg != NULL）
    }
    else
    {
        log_err(false, "[%s]The file is not JPEG Data!line:%d\n",__FUNCTION__,__LINE__);
    }
	
    return retVal;
}


