#ifndef __MINI264_HEADER__
#define __MINI264_HEADER__ 

#pragma pack(push,1)
#pragma pack(pop)
typedef int (bmpcallback)(void *p1, void *p2, unsigned char *bmp, int len);
#ifdef __cplusplus
extern "C" {
#endif 

/*bmpcallback
decode got bitmap callback function
===parameter===
p1 -- callback parameter 1
p2 -- callback parameter 2
bmp -- bitmap buffer address
len -- bitmap buffer length
*/

//typedef void (simd_func_type)(uint8_t **dst, const uint8_t **src, int len);

/*Gw_m264_init
initial mini h264 decode library
called only once at startup
*/
void Gw_m264_init();

/*m264_final
clean up h264 deocde library
called only once programe exit
*/
void Gw_m264_clean();

/*Gw_m264_new
create mini h264 decode instance
===parameter===
angle -- output bitmap rotate angle
bmpcallback -- callback function when decode got a bitmap
p1 --  first call back parameter
p2 -- second call back parameter
-----------------
return value: mini h264 context if success, else NULL
*/
void *Gw_m264_new(int angle);

/*Gw_m264_del
destroy mini h264 decode instance
===parameter===
ctx -- mini h264 instance, created by Gw_m264_new
*/
void Gw_m264_del(void *ctx);

/*Gw_m264_deocde
decode h264 frame
===parameter===
out -- output buffer
in  -- input buffer
in_size -- input buffer length
ctx -- mini h264 instance, created by Gw_m264_new
-----------------
return value: 
    > 0  , got a bitmap and return value is the length of bitmap
    ==0 ,  got no bitmap
    < 0  , decode failed
*/
int Gw_m264_deocde(unsigned char *out, const unsigned char *in, int in_size, void *ctx);

#ifdef __cplusplus
}
#endif 

#endif
