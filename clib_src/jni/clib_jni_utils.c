#include "clib_jni.h"
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include <android/bitmap.h>

#define min3v(v1, v2, v3)   ((v1)>(v2)? ((v2)>(v3)?(v3):(v2)):((v1)>(v3)?(v3):(v1)))  

#define max3v(v1, v2, v3)   ((v1)<(v2)? ((v2)<(v3)?(v3):(v2)):((v1)<(v3)?(v3):(v1))) 
 
#define clamp(a,min,max) \
    ({__typeof__ (a) _a__ = (a); \
      __typeof__ (min) _min__ = (min); \
      __typeof__ (max) _max__ = (max); \
      _a__ < _min__ ? _min__ : _a__ > _max__ ? _max__ : _a__; })
	  
// Based heavily on http://vitiy.info/Code/stackblur.cpp
// See http://vitiy.info/stackblur-algorithm-multi-threaded-blur-for-cpp/
// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
static unsigned short const stackblur_mul[255] =
{
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static unsigned char const stackblur_shr[255] =
{
        9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
        17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
        19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};
	 
typedef struct
{
u_int8_t red;        // [0,255]
u_int8_t green;      // [0,255]
u_int8_t blue;        // [0,255]
} COLOR_RGB;

typedef struct
{
  float hue;        // [0,360]
  float saturation;    // [0,100]
  float luminance;    // [0,100]
} COLOR_HSL;

// Converts RGB to HSL  
static void RGBtoHSL(const COLOR_RGB *rgb, COLOR_HSL *hsl)  
{  
 float h=0, s=0, l=0;  
 // normalizes red-green-blue values  
 float r = rgb->red/255.0f;  
 float g = rgb->green/255.0f;  
 float b = rgb->blue/255.0f;  
 float maxVal = max3v(r, g, b);  
 float minVal = min3v(r, g, b);  
  
 // hue  
 if(maxVal == minVal)  
 {  
  h = 0; // undefined  
 }  
 else if(maxVal==r && g>=b)  
 {  
  h = 60.0f*(g-b)/(maxVal-minVal);  
 }  
 else if(maxVal==r && g<b)  
 {  
  h = 60.0f*(g-b)/(maxVal-minVal) + 360.0f;  
 }  
 else if(maxVal==g)  
 {  
  h = 60.0f*(b-r)/(maxVal-minVal) + 120.0f;  
 }  
 else if(maxVal==b)  
 {  
  h = 60.0f*(r-g)/(maxVal-minVal) + 240.0f;  
 }  
  
 // luminance  
 l = (maxVal+minVal)/2.0f;  
  
 // saturation  
 if(l == 0 || maxVal == minVal)  
 {  
  s = 0;  
 }  
 else if(0<l && l<=0.5f)  
 {  
  s = (maxVal-minVal)/(maxVal+minVal);  
 }  
 else if(l>0.5f)  
 {  
  s = (maxVal-minVal)/(2 - (maxVal+minVal)); //(maxVal-minVal > 0)?  
 }  
  
 hsl->hue = (h>360)? 360 : ((h<0)?0:h);  
 hsl->saturation = ((s>1)? 1 : ((s<0)?0:s))*100;  
 hsl->luminance = ((l>1)? 1 : ((l<0)?0:l))*100;  
}  
// Converts HSL to RGB, 开源函数
static void HSLtoRGB(const COLOR_HSL *hsl, COLOR_RGB *rgb) 
{
  float h = hsl->hue;          // h must be [0, 360]
  float s = hsl->saturation/100.f;  // s must be [0, 1]
  float l = hsl->luminance/100.f;    // l must be [0, 1]
  float R, G, B;
  int i;

  if(hsl->saturation == 0)
  {
    // achromatic color (gray scale)
    R = G = B = l*255.f;
  }
  else
  {
    float q = (l<0.5f)?(l * (1.0f+s)):(l+s - (l*s));
    float p = (2.0f * l) - q;
    float Hk = h/360.0f;
    float T[3];
    T[0] = Hk + 0.3333333f;  // Tr  0.3333333f=1.0/3.0
    T[1] = Hk;        // Tb
    T[2] = Hk - 0.3333333f;  // Tg
    for(i=0; i<3; i++)
    {
      if(T[i] < 0) T[i] += 1.0f;
      if(T[i] > 1) T[i] -= 1.0f;
      if((T[i]*6) < 1)
      {
        T[i] = p + ((q-p)*6.0f*T[i]);
      }
      else if((T[i]*2.0f) < 1) //(1.0/6.0)<=T[i] && T[i]<0.5
      {
        T[i] = q;
      }
      else if((T[i]*3.0f) < 2) // 0.5<=T[i] && T[i]<(2.0/3.0)
      {
        T[i] = p + (q-p) * ((2.0f/3.0f) - T[i]) * 6.0f;
      }
      else T[i] = p;
    }
    R = T[0]*255.0f;
    G = T[1]*255.0f;
    B = T[2]*255.0f;
  }
  
  rgb->red = (u_int8_t)((R>255)? 255 : ((R<0)?0 : R));
  rgb->green = (u_int8_t)((G>255)? 255 : ((G<0)?0 : G));
  rgb->blue = (u_int8_t)((B>255)? 255 : ((B<0)?0 : B));
}

static int min_channel(u_int8_t * channel){
	int i, val = 255;

	for (i = 0; i < 5; i++) {
		if (val > channel[i])
			val = channel[i];
	}

	return val;
}

static int lede_led_do_state(float h, float s, float l)
{
	COLOR_RGB   rgb;
	COLOR_HSL   hsl;
	u_int8_t	channel_val[5];
	int argb = 0;
	
	hsl.hue = h;
	hsl.saturation = s;
	hsl.luminance = l;

	/*HSL到RGB转换，利用开源的算法*/
	HSLtoRGB(&hsl, &rgb);
	/*HSL到RGB转换完*/
	
	argb |= 0xff000000;
	argb |= (((int)rgb.red)<<16);
	argb |= (((int)rgb.green)<<8);
	argb |= ((int)rgb.blue);
	
	return argb;
}

/// Stackblur algorithm body
void stackblurJob(unsigned char* src,                ///< input image data
                  unsigned int w,                    ///< image width
                  unsigned int h,                    ///< image height
                  unsigned int radius,               ///< blur intensity (should be in 2..254 range)
                  int cores,                         ///< total number of working threads
                  int core,                          ///< current thread number
                  int step                           ///< step of processing (1,2)
                  )
{
    unsigned int x, y, xp, yp, i;
    unsigned int sp;
    unsigned int stack_start;
    unsigned char* stack_ptr;

    unsigned char* src_ptr;
    unsigned char* dst_ptr;

    unsigned long sum_r;
    unsigned long sum_g;
    unsigned long sum_b;
    unsigned long sum_in_r;
    unsigned long sum_in_g;
    unsigned long sum_in_b;
    unsigned long sum_out_r;
    unsigned long sum_out_g;
    unsigned long sum_out_b;

    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int w4 = w * 4;
    unsigned int div = (radius * 2) + 1;
    unsigned int mul_sum = stackblur_mul[radius];
    unsigned char shr_sum = stackblur_shr[radius];
    unsigned char stack[div * 3];

    if (step == 1)
    {
        int minY = core * h / cores;
        int maxY = (core + 1) * h / cores;

        for(y = minY; y < maxY; y++)
        {
            sum_r = sum_g = sum_b =
            sum_in_r = sum_in_g = sum_in_b =
            sum_out_r = sum_out_g = sum_out_b = 0;

            src_ptr = src + w4 * y; // start of line (0,y)

            for(i = 0; i <= radius; i++)
            {
                stack_ptr    = &stack[ 3 * i ];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                sum_r += src_ptr[0] * (i + 1);
                sum_g += src_ptr[1] * (i + 1);
                sum_b += src_ptr[2] * (i + 1);
                sum_out_r += src_ptr[0];
                sum_out_g += src_ptr[1];
                sum_out_b += src_ptr[2];
            }


            for(i = 1; i <= radius; i++)
            {
                if (i <= wm) src_ptr += 4;
                stack_ptr = &stack[ 3 * (i + radius) ];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
            }


            sp = radius;
            xp = radius;
            if (xp > wm) xp = wm;
            src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
            dst_ptr = src + y * w4; // img.pix_ptr(0, y);
            for(x = 0; x < w; x++)
            {
                int alpha = dst_ptr[3];
                dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr += 4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;

                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;
                stack_ptr = &stack[3 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];

                if(xp < wm)
                {
                    src_ptr += 4;
                    ++xp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_r    += sum_in_r;
                sum_g    += sum_in_g;
                sum_b    += sum_in_b;

                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp*3];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
                sum_in_r  -= stack_ptr[0];
                sum_in_g  -= stack_ptr[1];
                sum_in_b  -= stack_ptr[2];
            }

        }
    }

    // step 2
    if (step == 2)
    {
        int minX = core * w / cores;
        int maxX = (core + 1) * w / cores;

        for(x = minX; x < maxX; x++)
        {
            sum_r =    sum_g =    sum_b =
            sum_in_r = sum_in_g = sum_in_b =
            sum_out_r = sum_out_g = sum_out_b = 0;

            src_ptr = src + 4 * x; // x,0
            for(i = 0; i <= radius; i++)
            {
                stack_ptr    = &stack[i * 3];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                sum_r           += src_ptr[0] * (i + 1);
                sum_g           += src_ptr[1] * (i + 1);
                sum_b           += src_ptr[2] * (i + 1);
                sum_out_r       += src_ptr[0];
                sum_out_g       += src_ptr[1];
                sum_out_b       += src_ptr[2];
            }
            for(i = 1; i <= radius; i++)
            {
                if(i <= hm) src_ptr += w4; // +stride

                stack_ptr = &stack[3 * (i + radius)];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
            }

            sp = radius;
            yp = radius;
            if (yp > hm) yp = hm;
            src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
            dst_ptr = src + 4 * x;               // img.pix_ptr(x, 0);
            for(y = 0; y < h; y++)
            {
                int alpha = dst_ptr[3];
                dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
                dst_ptr += w4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;

                stack_start = sp + div - radius;
                if(stack_start >= div) stack_start -= div;
                stack_ptr = &stack[3 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];

                if(yp < hm)
                {
                    src_ptr += w4; // stride
                    ++yp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_r    += sum_in_r;
                sum_g    += sum_in_g;
                sum_b    += sum_in_b;

                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp*3];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
                sum_in_r  -= stack_ptr[0];
                sum_in_g  -= stack_ptr[1];
                sum_in_b  -= stack_ptr[2];
            }
        }
    }
}

JNIEXPORT jint JNICALL
NAME(ClColorHSL2RGB)(JNIEnv* env, jobject this, jfloat h, jfloat s, jfloat l)
{
	return lede_led_do_state(h, s, l);
}

JNIEXPORT void JNICALL
NAME(ClColorRGB2HSL)(JNIEnv* env, jobject this, jint r, jint g, jint b, jintArray  array_hsl)
{
	COLOR_RGB   rgb={r, g, b};
	COLOR_HSL   hsl;
	jint *arrs;
	
	RGBtoHSL(&rgb, &hsl);

	arrs = (*env)->GetIntArrayElements(env, array_hsl, NULL);
	
	arrs[0] = hsl.hue;
	arrs[1] = hsl.saturation;
	arrs[2] = hsl.luminance;
	
	(*env)->ReleaseIntArrayElements(env, array_hsl, arrs, 0); //需要确认

}
JNIEXPORT jint JNICALL
NAME(ClGetStringUTFCharsLength)(JNIEnv* env, jobject this, jobject string)
{
	const char *pstr = NULL;
	int len = 0;

	if(string == NULL)
		return len;
	
	pstr = (*env)->GetStringUTFChars(env, string, NULL);
	len = strlen(pstr);
	(*env)->ReleaseStringUTFChars(env, string, pstr);
	
	return len;
}

JNIEXPORT void JNICALL
NAME(ClBlurBitmap)(JNIEnv* env, jclass clzz, jobject bitmapOut, jint radius) {
	// Properties
    AndroidBitmapInfo   infoOut;
    void*               pixelsOut;

    int ret;

    // Get image info
    if ((ret = AndroidBitmap_getInfo(env, bitmapOut, &infoOut)) != 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    // Check image
    if (infoOut.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888!");
        LOGE("==> %d", infoOut.format);
        return;
    }

    // Lock all images
    if ((ret = AndroidBitmap_lockPixels(env, bitmapOut, &pixelsOut)) != 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    int h = infoOut.height;
    int w = infoOut.width;

	//作者考虑了多线程 分布处理的情况，这里忽略
    stackblurJob((unsigned char*)pixelsOut, w, h, radius, 1, 0, 1);
	stackblurJob((unsigned char*)pixelsOut, w, h, radius, 1, 0, 2);

    // Unlocks everything
    AndroidBitmap_unlockPixels(env, bitmapOut);
}