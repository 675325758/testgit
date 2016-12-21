#include "clib_jni.h"
#include <assert.h>
// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

/************************************************************************************
		video interface 
 ************************************************************************************/

typedef struct {
	/* 连接方式 */
	u_int8_t is_tcp; /* tcp or udp */
	u_int8_t resv[7];	
} video_stat_priv_t;


static u_int32_t video_handle;

/************************************************************************************
 使用OpenSLES播放音频
 ************************************************************************************/
// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;

/************************************************************************************
 使用OpenGL播放视频
 ************************************************************************************/
static bool isRecvData;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


void createEngine()
{
    SLresult result;
    
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                                                                                   outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
    
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
}

// create buffer queue audio player
void createBufferQueueAudioPlayer()
{
    SLresult result;
    
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
        /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
        /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                             &bqPlayerEffectSend);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
#endif
    
    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    
    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
}

static void shutdown2()
{
    
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerMuteSolo = NULL;
        bqPlayerVolume = NULL;
    }
    
    // destroy audio recorder object, and invalidate all associated interfaces
    /*if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
    }*/
    
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    
}

void VideoCallback(u_int32_t event, void *user_handle, void *callback_handle)
{
    if(event == VE_GET_PICTURE) {
        isRecvData = true;
        /*void *pic;
        u_int32_t len;
        if (cl_video_get_picture(video_handle, &pic, &len) != RS_OK || len == 0) {
            LOGE("cl_video_get_picture failed");
            return;
        }
        u_int32_t Width, Heigth;
        if(parse_pic((u_int8_t *)pic, len, &Width, &Heigth)== RS_OK) {
            LOGE("VE_GET_PICTURE, Width:%d, Heigth:%d", Width, Heigth);
        }*/
    } else if(event == VE_GET_SOUND) {
        cl_sound_data_t sd;
        if (cl_audio_get_sound(video_handle, &sd) != RS_OK) {
            return;
        }
        SLresult result;
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, (u_int8_t *)sd.data, sd.len);
        if (SL_RESULT_SUCCESS != result) {
            return;
        }
        LOGE("VE_GET_SOUND");
    }
}

JNIEXPORT jint JNICALL
NAME(ClVideoStart)(JNIEnv* env, jobject this, int slave_handle, jint callback_handle)
{
#if 0
    isRecvData = false;
    createEngine();
    createBufferQueueAudioPlayer();
    video_handle = slave_handle;
    setVideoCallback(VideoCallback);
#endif
	return cl_video_start(slave_handle, nactivCallback, (void *)(jlong)callback_handle);
}

JNIEXPORT jint JNICALL
NAME(ClVideoStop)(JNIEnv* env, jobject this, jint slave_handle)
{
#if 0
    shutdown();
#endif
	return cl_video_stop(slave_handle);
}


JNIEXPORT jbyteArray JNICALL
NAME(ClVideoGetPicture)(JNIEnv* env, jobject this, jint slave_handle)
{
	void *pic;
	u_int32_t len;
	jbyteArray  jbarray;

	if (cl_video_get_picture(slave_handle, &pic, &len) != RS_OK || len == 0) {
		LOGE("cl_video_get_picture failed");
		return NULL;
	}

	jbarray = (*env)->NewByteArray(env, len);

	(*env)->SetByteArrayRegion(env, jbarray, 0, len, (jbyte *)pic);
	//(*env)->DeleteLocalRef(env, jbarray);

	return jbarray;
}

unsigned int vbo[2];
float positions[12] = {1,-1,0, 1,1,0, -1,-1,0, -1,1,0};
short indices  [4]  = {0,1,2,3};

JNIEXPORT void JNICALL
NAME(ClVideoSurfaceCreated)(JNIEnv* env, jobject this)
{
     //生成两个缓存区对象
     glGenBuffers (2, vbo);
     //绑定第一个缓存对象
     glBindBuffer (GL_ARRAY_BUFFER, vbo[0]);
     //创建和初始化第一个缓存区对象的数据
     glBufferData (GL_ARRAY_BUFFER, 4*12, positions, GL_STATIC_DRAW);
     //绑定第二个缓存对象
     glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
     //创建和初始化第二个缓存区对象的数据
     glBufferData (GL_ELEMENT_ARRAY_BUFFER, 2*4, indices, GL_STATIC_DRAW);
}

JNIEXPORT void JNICALL
NAME(ClVideoSurfaceChanged)(JNIEnv* env, jobject this, jint width, jint height)
{
     //图形最终显示到屏幕的区域的位置、长和宽
     glViewport(0,0,width,height);
     //指定矩阵
     glMatrixMode(GL_PROJECTION);
     //将当前的矩阵设置为glMatrixMode指定的矩阵
     glLoadIdentity ();
     glOrthof(-2, 2, -2, 2, -2, 2);

     
}

JNIEXPORT void JNICALL
NAME(ClVideoSurfaceDrawFrame)(JNIEnv* env, jobject this)
{
    
    if(isRecvData) {
        void *pic;
        u_int32_t len;
        if (cl_video_get_picture(video_handle, &pic, &len) != RS_OK || len == 0) {
            LOGE("cl_video_get_picture failed");
            return;
        }
    }
    if(isRecvData) {
        void *pic;
        u_int32_t len;
        if (cl_video_get_picture(video_handle, &pic, &len) != RS_OK || len == 0) {
            LOGE("cl_video_get_picture failed");
            return;
        }
        u_int32_t Width, Heigth;
        if(parse_pic((u_int8_t *)pic, len, &Width, &Heigth) == RS_OK) {
            //u_int8_t *data = get_pic_data((u_int8_t *)pic, len);
            
            u_int8_t *data;
            get_pic_data((u_int8_t *)pic, len, &data, Width, Heigth);
            
            if(data) {
                //启用顶点设置功能，之后必须要关闭功能
                glEnableClientState (GL_VERTEX_ARRAY);
                //清屏
                glClearColor(0.0,0.0,0.0,0.0);
                glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glMatrixMode (GL_MODELVIEW);
                glLoadIdentity ();
                glBindBuffer    (GL_ARRAY_BUFFER, vbo[0]);
                //定义顶点坐标
                glVertexPointer (3, GL_FLOAT, 0, 0);
                glBindBuffer    (GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
                //按照参数给定的值绘制图形
                glDrawElements  (GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
                
                
                /*
                 // 创建纹理
                 glGenTextures(1, &texture[0]);
                 // 使用来自位图数据生成 的典型纹理
                 glBindTexture(GL_TEXTURE_2D, texture[0]);
                 // 生成纹理
                 glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // 线性滤波
                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
                
                
                GLuint texName;
                 glShadeModel(GL_FLAT);
                 glEnable(GL_DEPTH_TEST);
                 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                 glGenTextures(1, &texName);
                 glBindTexture(GL_TEXTURE_2D, texName);
                 
                 
                 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Linear Filtered
                 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // Linear Filtered
                 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                 //glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                 //glTexImage2D(GL_TEXTURE_2D,0,3,Width,Heigth,0,GL_RGB,GL_UNSIGNED_BYTE,data);
                 //glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                 glEnable(GL_TEXTURE_2D);
                
                //关闭顶点设置功能
                glDisableClientState(GL_VERTEX_ARRAY);
                
            }
            LOGE("VE_GET_PICTURE, Width:%d, Heigth:%d", Width, Heigth);
        }
        
    }
}


JNIEXPORT jobject JNICALL
NAME(ClVideoGetStat)(JNIEnv* env, jobject this, jint slave_handle)
{
	cl_video_stat_t stat;
	video_stat_priv_t *stat_priv_t;
	jbyteArray jbarray;
	jclass class_vs;
	jobject obj_vs;
	jfieldID fid;

	if (cl_video_get_stat(slave_handle, &stat) != RS_OK) {
		LOGE("cl_video_get_stat failed");
		return NULL;
	}

	class_vs = (*env)->FindClass(env, CLASS_VIDEO_STAT);
	obj_vs = (*env)->AllocObject(env, class_vs);
	
	fid = (*env)->GetFieldID(env, class_vs, "is_custom_quality", "Z");
	(*env)->SetBooleanField(env, obj_vs, fid, stat.is_custom_quality);
	
	fid = (*env)->GetFieldID(env, class_vs, "quality", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.quality);

	fid = (*env)->GetFieldID(env, class_vs, "width", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.width);

	fid = (*env)->GetFieldID(env, class_vs, "height", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.height);

	fid = (*env)->GetFieldID(env, class_vs, "fps", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.fps);

	fid = (*env)->GetFieldID(env, class_vs, "bitrate", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.bitrate);

	fid = (*env)->GetFieldID(env, class_vs, "client_count", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.client_count);
	
	stat_priv_t = (video_stat_priv_t *)stat.resv;
	
	fid = (*env)->GetFieldID(env, class_vs, "is_tcp", "Z");
	(*env)->SetBooleanField(env, obj_vs, fid, stat_priv_t->is_tcp);

	jbarray = (*env)->NewByteArray(env, sizeof(stat.resv));
	(*env)->SetByteArrayRegion(env, jbarray, sizeof(video_stat_priv_t), sizeof(stat.resv) - sizeof(video_stat_priv_t), (jbyte *)stat.resv);

	fid = (*env)->GetFieldID(env, class_vs, "resv", "[B");
	(*env)->SetObjectField(env, obj_vs, fid, jbarray);

	//拷贝云台转速
	fid = (*env)->GetFieldID(env, class_vs, "rollSpeed", "I");
	(*env)->SetIntField(env, obj_vs, fid, stat.roll_speed);

	//拷贝视频饱和度相关信息到obj_vs中
	jclass class_video_saturation = (*env)->FindClass(env, CLASS_VIDEO_SATURATION);
	jobject obj_video_saturation = (*env)->AllocObject(env, class_video_saturation);

	jniCopyIntValue(env,class_video_saturation,"brightnessVal",obj_video_saturation,stat.vs_stat.brightness_val);
	jniCopyIntValue(env,class_video_saturation,"contrastVal",obj_video_saturation,stat.vs_stat.contrast_val);
	jniCopyIntValue(env,class_video_saturation,"saturationVal",obj_video_saturation,stat.vs_stat.saturation_val);
	jniCopyIntValue(env,class_video_saturation,"gainVal",obj_video_saturation,stat.vs_stat.gain_val);

	

	
	fid = (*env)->GetFieldID(env, class_vs, "vedioSaturation", "L" CLASS_VIDEO_SATURATION ";");
	(*env)->SetObjectField(env, obj_vs, fid, obj_video_saturation);
	(*env)->DeleteLocalRef(env, obj_video_saturation);
	
	//(*env)->DeleteLocalRef(env, jbarray);
	(*env)->DeleteLocalRef(env, class_vs);

	return obj_vs;
}

JNIEXPORT jint JNICALL
NAME(ClVideoPtzRoll)(JNIEnv* env, jobject this, jint slave_handle, jint left_right, jint up_down)
{
	return cl_video_ptz_roll(slave_handle, left_right, up_down);
}

JNIEXPORT jint JNICALL
NAME(ClVideoPtzRollStart)(JNIEnv* env, jobject this, jint slave_handle, jint left_right, jint up_down)
{
	return cl_video_ptz_roll_start(slave_handle, left_right, up_down);
}

JNIEXPORT jint JNICALL
NAME(ClVideoPtzRollStop)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_video_ptz_roll_stop(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClVideoSetQuality)(JNIEnv* env, jobject this, jint slave_handle, 
	jint quality, jint width, jint height, jint fps)
{
	cl_video_quality_t q;

	q.quality = quality;
	q.width = width;
	q.height = height;
	q.fps = fps;
	
	return cl_video_set_quality(slave_handle, &q);
}

JNIEXPORT jint JNICALL
NAME(ClAudioRequestSpeek)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_audio_request_speek(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClAudioReleaseSpeed)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_audio_release_speek(slave_handle);
}

JNIEXPORT jobject JNICALL
NAME(ClAudioGetSound)(JNIEnv* env, jobject this, jint slave_handle)
{
	cl_sound_data_t sd;
	jobject object;
	jclass clazz;
	jfieldID fid;
	jbyteArray jbarray;
	
	if (cl_audio_get_sound(slave_handle, &sd) != RS_OK)
		return NULL;

	clazz = (*env)->FindClass(env, CLASS_SOUND_DATA);
	object = (*env)->AllocObject(env, clazz);

	fid = (*env)->GetFieldID(env, clazz, "channels", "I");
	(*env)->SetIntField(env, object, fid, sd.channels);

	fid = (*env)->GetFieldID(env, clazz, "bits", "I");
	(*env)->SetIntField(env, object, fid, sd.bits);

	fid = (*env)->GetFieldID(env, clazz, "samples", "I");
	(*env)->SetIntField(env, object, fid, sd.samples);

	jbarray = (*env)->NewByteArray(env, sd.len);
	(*env)->SetByteArrayRegion(env, jbarray, 0, sd.len, (jbyte *)sd.data);
  
	fid = (*env)->GetFieldID(env, clazz, "data", "[B");
	(*env)->SetObjectField(env, object, fid, jbarray);

	//(*env)->DeleteLocalRef(env, jbarray);
	(*env)->DeleteLocalRef(env, clazz);

	return object;
}

JNIEXPORT jint JNICALL
NAME(ClAudioPutSound)(JNIEnv* env, jobject this, jint slave_handle, jobject object)
{
	cl_sound_data_t sd;
	jclass clazz;
	jobject byte_array;
	jfieldID fid;
	RS res;

	memset(&sd, 0, sizeof(sd));
	
	clazz = (*env)->FindClass(env, CLASS_SOUND_DATA);

	fid = (*env)->GetFieldID(env, clazz, "channels", "I");
	sd.channels = (*env)->GetIntField(env, object, fid);

	fid = (*env)->GetFieldID(env, clazz, "bits", "I");
	sd.bits = (*env)->GetIntField(env, object, fid);

	fid = (*env)->GetFieldID(env, clazz, "samples", "I");
	sd.samples= (*env)->GetIntField(env, object, fid);
  
	fid = (*env)->GetFieldID(env, clazz, "data", "[B");
	byte_array = (*env)->GetObjectField(env, object, fid);

	sd.data = (void *)((*env)->GetByteArrayElements(env, byte_array, NULL));
	sd.len = (*env)->GetArrayLength(env, byte_array);

	res = cl_audio_put_sound(slave_handle, &sd);
	(*env)->ReleaseByteArrayElements(env, byte_array, (jbyte*)sd.data, JNI_ABORT);	

	(*env)->DeleteLocalRef(env, clazz);

	return res;
}

JNIEXPORT jint JNICALL
NAME(ClVideoFlip)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_video_flip(slave_handle);
}

static SLVolumeItf getVolume()
{
    return bqPlayerVolume;
}

JNIEXPORT jint JNICALL
NAME(ClSetMuteUriAudioPlayer)(JNIEnv* env, jclass clazz, jboolean mute)
{
    SLresult result;
    SLVolumeItf volumeItf = getVolume();
    if (NULL != volumeItf) {
        result = (*volumeItf)->SetMute(volumeItf, mute);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;
        return RS_OK;
    } else {
        return RS_ERROR;
    }
    
}

/************************************************************************************
		定时录像相关接口
 ************************************************************************************/

static jobject CopyVrtItem(JNIEnv* env, jclass clazz, cl_vrt_item_t *item)
{
	jobject obj;
	jfieldID fid;
	jstring str;

	obj = (*env)->AllocObject(env, clazz);

	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	(*env)->SetIntField(env, obj, fid, item->id);

	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	(*env)->SetBooleanField(env, obj, fid, item->enable);

	fid = (*env)->GetFieldID(env, clazz, "is_once", "Z");
	(*env)->SetBooleanField(env, obj, fid, item->is_once);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	(*env)->SetIntField(env, obj, fid, item->wday);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	(*env)->SetIntField(env, obj, fid, item->hour);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	(*env)->SetIntField(env, obj, fid, item->minute);
	
	fid = (*env)->GetFieldID(env, clazz, "last", "I");
	(*env)->SetIntField(env, obj, fid, item->duration);

	fid = (*env)->GetFieldID(env, clazz, "location_time", "I");
	(*env)->SetIntField(env, obj, fid, item->location_time);

	jniCopyString(env, clazz, "name", obj, item->name);

	return obj;
}

static void CopyVideoInfo(JNIEnv* env, jclass class_video_info, jobject obj_video_info, cl_video_info_t *info)
{
	int i;
	jclass clazz;
	jfieldID fid;
	jobject *obj;
	jobjectArray array = NULL;
	
	fid = (*env)->GetFieldID(env, class_video_info, "has_establish", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->has_establish);
	
	fid = (*env)->GetFieldID(env, class_video_info, "has_ptz", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->has_ptz);
	
	
	fid = (*env)->GetFieldID(env, class_video_info, "is_h264", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->is_h264);
	
	fid = (*env)->GetFieldID(env, class_video_info, "has_audio", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->has_audio);
	
	fid = (*env)->GetFieldID(env, class_video_info, "has_audio_speek", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->has_audio_speek);

	fid = (*env)->GetFieldID(env, class_video_info, "record_enable", "Z");
	(*env)->SetBooleanField(env, obj_video_info, fid, info->record_enable);

	fid = (*env)->GetFieldID(env, class_video_info, "record_status", "I");
	(*env)->SetIntField(env, obj_video_info, fid, info->record_status);

	fid = (*env)->GetFieldID(env, class_video_info, "num_timer", "I");
	(*env)->SetIntField(env, obj_video_info, fid, info->num_timer);

	// 拷贝定时器数组
	clazz = (*env)->FindClass(env, CLASS_VRT_ITEM);
	if (info->num_timer > 0) {
		array = (*env)->NewObjectArray(env, info->num_timer, clazz, NULL);
	}
	
	for (i = 0; i < info->num_timer; i++) {
		obj = CopyVrtItem(env, clazz, info->timer[i]);
		(*env)->SetObjectArrayElement(env, array, i, obj);
	}
	
	fid = (*env)->GetFieldID(env, class_video_info, "timer", "[L" CLASS_VRT_ITEM ";");
	(*env)->SetObjectField(env, obj_video_info, fid, array);
	(*env)->DeleteLocalRef(env, clazz);
}

JNIEXPORT jobject JNICALL
NAME(ClVideoGetInfo)(JNIEnv* env, jobject this, jint slave_handle, jint tz)
{
	cl_video_info_t *info;
	jclass class_video_info;
	jobject obj_video_info;

	if ((info = cl_video_info_get(slave_handle, tz)) == NULL) {
		return NULL;
	}

	class_video_info = (*env)->FindClass(env, CLASS_VIDEO_INFO);
	obj_video_info = (*env)->AllocObject(env, class_video_info);
	
	CopyVideoInfo(env, class_video_info, obj_video_info, info);
	
	cl_video_info_free(info);
	(*env)->DeleteLocalRef(env, class_video_info);
	return obj_video_info;
}


/*
	功能:
		录像的总开关
	输入参数:
		@slave_handle: 设备句柄
		@on: 1表示使能录像规则，0表示禁止录像规则
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/


JNIEXPORT jint JNICALL
NAME(ClVideoRecTimerTurn)(JNIEnv* env, jobject this, jint slave_handle, jboolean on)
{
	RS ret = RS_ERROR;
	ret = cl_video_rec_timer_turn_on(slave_handle, on);
	return ret;
}


/*
	功能:
		删除定时录像规则
	输入参数:
		@slave_handle: 设备句柄
		@id: 要删除的规则的id
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

JNIEXPORT jint JNICALL
NAME(ClVideoRecTimerDel)(JNIEnv* env, jobject this, jint slave_handle, jint id)
{
	RS ret = RS_ERROR;
	ret = cl_video_rec_timer_del(slave_handle, id);
	return ret;
}

/*
	功能:
		添加定时定时录像规则
	输入参数:
		@slave_handle: 设备句柄
		@timer: 要添加的定时录像规则。
			timer->name必须为UTF-8格式，长度小于64字节。
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/


JNIEXPORT jint JNICALL
NAME(ClVideoRecTimerAdd)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_timer, jint tz)
{
	jint ret = 0;
	cl_vrt_item_t pt;
	jclass clazz;
	jfieldID fid;
	jobject obj_name;
	jstring str;
	

	memset(&pt, 0, sizeof(cl_vrt_item_t));
	clazz = (*env)->FindClass(env, CLASS_VRT_ITEM);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	pt.id = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	pt.hour = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	pt.minute = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	pt.wday= (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	pt.enable = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "last", "I");
	pt.duration= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "location_time", "I");
	pt.location_time = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "is_once", "Z");
	pt.is_once = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_name = (*env)->GetObjectField(env, obj_timer, fid);
	pt.name = (char *)((*env)->GetStringUTFChars(env, obj_name, NULL));
	

	ret = cl_video_rec_timer_add( slave_handle, &pt, tz);

	(*env)->ReleaseStringUTFChars(env, obj_name, pt.name);
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

/*
	功能:
		修改定时录像规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@timer: 要修改的定时录像规则。
			timer->name必须为UTF-8格式，长度小于64字节。
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

JNIEXPORT jint JNICALL
NAME(ClVideoRecTimerModify)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_timer, jint tz)
{
	jint ret = 0;
	cl_vrt_item_t pt;
	jclass clazz;
	jfieldID fid;
	jobject obj_name;
	jstring str;
	

	memset(&pt, 0, sizeof(cl_vrt_item_t));
	clazz = (*env)->FindClass(env, CLASS_VRT_ITEM);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	pt.id = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	pt.hour = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	pt.minute = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	pt.wday= (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	pt.enable = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "last", "I");
	pt.duration= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "location_time", "I");
	pt.location_time = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "is_once", "Z");
	pt.is_once = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_name = (*env)->GetObjectField(env, obj_timer, fid);
	pt.name = (char *)((*env)->GetStringUTFChars(env, obj_name, NULL));
	

	ret = cl_video_rec_timer_modify(slave_handle, &pt, tz);

	(*env)->ReleaseStringUTFChars(env, obj_name, pt.name);
	(*env)->DeleteLocalRef(env, clazz);
	
	return ret;
}

/*
 CLIB_API RS cl_video_set_saturation(cl_handle_t slave_handle, cl_video_saturation_t* vs);
    功能:
        设置视频饱和度
    输入参数:
        @vs: 视频饱和度参数配置
     输出参数:
     无
     返回:
     无
*/
JNIEXPORT jint JNICALL
NAME(ClVideoSetSaturation)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_video_saturation)
{
	jint ret;
	jclass clazz;
	jfieldID fid;
	cl_video_saturation_t cl_video_saturation;

	memset(&cl_video_saturation, 0, sizeof(cl_video_saturation_t));
	clazz = (*env)->FindClass(env, CLASS_VIDEO_SATURATION);

	fid = (*env)->GetFieldID(env, clazz, "brightnessVal", "I");
	cl_video_saturation.brightness_val = (*env)->GetIntField(env, obj_video_saturation, fid);

	fid = (*env)->GetFieldID(env, clazz, "contrastVal", "I");
	cl_video_saturation.contrast_val = (*env)->GetIntField(env, obj_video_saturation, fid);

	fid = (*env)->GetFieldID(env, clazz, "saturationVal", "I");
	cl_video_saturation.saturation_val = (*env)->GetIntField(env, obj_video_saturation, fid);

	fid = (*env)->GetFieldID(env, clazz, "gainVal", "I");
	cl_video_saturation.gain_val = (*env)->GetIntField(env, obj_video_saturation, fid);

	ret = cl_video_set_saturation(slave_handle, &cl_video_saturation);

	(*env)->DeleteLocalRef(env, clazz);

	return ret;
}

/*
 CLIB_API RS cl_video_set_roll_speed(cl_handle_t slave_handle,u_int8_t speed);
   功能:
	设置视频云台转动速度
   输入参数:
	@speed:视频云台转速，0-100
   输出参数:
	无
   返回:
	无
 */
JNIEXPORT jint JNICALL
NAME(ClVideoSetRollSpeed)(JNIEnv* env, jobject this, jint slave_handle, jint speed)
{
	return cl_video_set_roll_speed(slave_handle, speed);
}


/*
 功能:
    查询视频有多少录像(只支持按天获取录像）
 输入参数:
    @begin_time 开始时间,0表示当天
 输出参数:
    无
 返回:
    无
 */
JNIEXPORT jint JNICALL
NAME(ClQueryVtapList)
(JNIEnv* env, jobject this, jint slave_handle, jint begin_time)
{
	return cl_query_vtap_list(slave_handle, begin_time);
}

/*
 功能：
    获取上次cl_query_vtap_list的结果
 参数IN：
    slave_handle:录像设备句柄。
 参数OUT：
    无
 返回：
    cl_vtap_list_t：视频列表
 */
JNIEXPORT jobject JNICALL
NAME(ClGetVtapListData)
(JNIEnv* env, jobject this, jint slave_handle)
{
	cl_vtap_list_t* list_data = NULL;
	jclass class_data = NULL;
	jobject obj_array = NULL;
	jobject obj_data = NULL;
	int i = 0;
	
	list_data = cl_get_vtap_list_data(slave_handle);
	
	if(!list_data) {
		return NULL;
	}
	
	class_data = (*env)->FindClass(env, CLASS_VTAP_INFO);
	
	if(!class_data) {
		return NULL;
	}
	if(list_data->total_num > 0) {
		obj_array = (*env)->NewObjectArray(env, list_data->total_num, class_data, NULL);
		for (i = 0; i < list_data->total_num; i++) {
			obj_data = (*env)->AllocObject(env, class_data);
			jniCopyIntValue(env, class_data, "begin_time", obj_data, list_data->vtap[i].begin_time);
			jniCopyIntValue(env, class_data, "duration", obj_data, list_data->vtap[i].duration);
			
			(*env)->SetObjectArrayElement(env, obj_array, i, obj_data);
			(*env)->DeleteLocalRef(env, obj_data);
		}
	}
	
	(*env)->DeleteLocalRef(env, class_data);
	cl_free_vtap_list_data(list_data);
	return obj_array;
}

JNIEXPORT jint JNICALL
NAME(ClVtapStart)
(JNIEnv* env, jobject this, jint slave_handle, jint begin_time, jint callback_handle)
{
	return cl_vtap_start(slave_handle, begin_time, nactivCallback, (void *)(jlong)callback_handle);
}

JNIEXPORT jint JNICALL
NAME(ClVtapStop)
(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_vtap_stop(slave_handle);
}

JNIEXPORT jobject JNICALL
NAME(ClVtapGetPicture)(JNIEnv* env, jobject this, jint slave_handle)
{
	void *pic;
	u_int32_t len, pic_time;
	jbyteArray jbarray;
	jclass class_picture;
	jobject obj_picture = NULL;
	jfieldID fid;
	
	class_picture = (*env)->FindClass(env, CLASS_VTAP_PICTURE);
	if(!class_picture) {
		return NULL;
	}
	if (cl_vtap_get_picture(slave_handle, &pic_time, &pic, &len) != RS_OK || len == 0) {
		LOGE("cl_vtap_get_picture failed");
		return NULL;
	}

	obj_picture = (*env)->AllocObject(env, class_picture);
	
	jniCopyIntValue(env, class_picture, "pic_time", obj_picture, pic_time);
	
	fid = (*env)->GetFieldID(env, class_picture, "pic", "[B");
	jbarray = (*env)->NewByteArray(env, len);
	(*env)->SetByteArrayRegion(env, jbarray, 0, len, (jbyte *)pic);
	(*env)->SetObjectField(env, obj_picture, fid, jbarray);
	
	
	(*env)->DeleteLocalRef(env, class_picture);
	SAFE_DEL_LOCAL_REF(jbarray);

	return obj_picture;
}