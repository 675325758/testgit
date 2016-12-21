#ifndef	__H264_DECORDE_H__
#define	__H264_DECORDE_H__

#ifdef __cplusplus
extern "C" {
#endif 
    
//#define REMOVE_H264_LIB

#ifndef REMOVE_H264_LIB
#include "mini264lib.h"
    
#endif

extern RS start_h264_decode(video_t *video);
extern void stop_h264_decode(video_t *video);


#ifdef __cplusplus
}
#endif 

#endif

