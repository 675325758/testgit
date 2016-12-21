#ifndef	 __VIDEO_RECORD_TIMER_H__
#define	__VIDEO_RECORD_TIMER_H__

#include "cl_video.h"

#ifdef __cplusplus
extern "C" {
#endif 

extern void vrt_do_timer_a(video_t *video, pkt_t *pkt);
extern void vrt_quick_query(video_t *video);
extern bool vrt_proc_notify(video_t *video, cl_notify_pkt_t *pkt, RS *ret);

#ifdef __cplusplus
}
#endif 

#endif

