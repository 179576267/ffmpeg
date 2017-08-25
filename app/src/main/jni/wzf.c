#include "header/com_wzf_ffmpeg_VideoUtils.h"
#include "./header/android_log.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>

//c线程类
#include <unistd.h>
#include <pthread.h>

//编码
#include "header/include/libavcodec/avcodec.h"
//封装格式处理
#include "header/include/libavformat/avformat.h"
//像素处理
#include "header/include/libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"

#include "libyuv.h"

#define MAX_AUDIO_FRME_SIZE 48000 * 4

JavaVM *javaVM;
jobject uuidutils_class_global;
jmethodID uuidutils_get_mid;

JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_decode(JNIEnv *env, jobject jthiz,
                                                             jstring input_jstr,
                                                             jstring output_jstr) {
    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    const char *output_cstr = (*env)->GetStringUTFChars(env, output_jstr, NULL);

    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOG_I_DEBUG("%s", "打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOG_I_DEBUG("%s", "获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        //根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if (pCodec == NULL) {
        LOG_I_DEBUG("%s", "无法解码");
        return;
    }

    //5.打开解码器
    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        LOG_I_DEBUG("%s", "解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *) yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodeCtx->width,
                   pCodeCtx->height);


    //输出文件
    FILE *fp_yuv = fopen(output_cstr, "wb");

    //用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            pCodeCtx->width, pCodeCtx->height, pCodeCtx->pix_fmt,
            pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    int len, got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if (got_frame) {
            //frame->yuvFrame (YUV420P)
            //转为指定的YUV420P像素帧
            sws_scale(sws_ctx,
                      frame->data, frame->linesize, 0, frame->height,
                      yuvFrame->data, yuvFrame->linesize);

            //向YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size = pCodeCtx->width * pCodeCtx->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size / 4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size / 4, fp_yuv);

            LOG_I_DEBUG("解码%d帧", framecount++);
        }

        av_free_packet(packet);
    }

    fclose(fp_yuv);

    av_frame_free(&frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
    (*env)->ReleaseStringUTFChars(env, output_jstr, output_cstr);
}


jstring JNICALL Java_com_wzf_ffmpeg_VideoUtils_getVideoInfo(JNIEnv *env, jobject jthiz,
                                                            jstring input_jstr) {
    //需要转码的视频文件(输入的视频文件)
    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    //1.注册所有组件,初始化信息
    av_register_all();

    //封装格式上下文，统领全局的结构体，保存了视频文件封装格式的相关信息，时长，分辨率，等等
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //pFormatCtx->duration;//时长
    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOG_I_DEBUG("%s", "无法打开输入视频文件");
        return (*env)->NewStringUTF(env, "无法打开输入视频文件");
    }
    //3.获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOG_I_DEBUG("%s", "无法获取视频文件信息");
        return (*env)->NewStringUTF(env, "无法获取视频文件信息");
    }

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_idx = -1;
    int i = 0;
    //number of streams
    for (; i < pFormatCtx->nb_streams; i++) {
        //流的类型
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            v_stream_idx = i;
            break;
        }
    }

    if (v_stream_idx == -1) {
        LOG_I_DEBUG("%s", "找不到视频流\n");
        return (*env)->NewStringUTF(env, "找不到视频流");
    }

    //只有知道视频的编码方式，才能够根据编码方式去找到解码器
    //获取视频流中的编解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[v_stream_idx]->codec;
    //4.根据编解码上下文中的编码id查找对应的解码
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    //（迅雷看看，找不到解码器，临时下载一个解码器）
    if (pCodec == NULL) {
        LOG_I_DEBUG("%s", "找不到解码器\n");
        return (*env)->NewStringUTF(env, "找不到解码器");
    }

    //5.打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOG_I_DEBUG("%s", "解码器无法打开\n");
        return (*env)->NewStringUTF(env, "解码器无法打开");
    }

    //输出视频信息
    LOG_I_DEBUG("视频的文件格式：%s", pFormatCtx->iformat->name);
    LOG_I_DEBUG("视频时长：%d", (pFormatCtx->duration) / 1000000);
    LOG_I_DEBUG("视频的宽高：%d,%d", pCodecCtx->width, pCodecCtx->height);
    LOG_I_DEBUG("解码器的名称：%s", pCodec->name);
    char result[1000];
    sprintf(result, "视频的文件格式：%s, \n视频时长：%lld, \n视频的宽高：%d,%d, \n 解码器的名称：%s",
            pFormatCtx->iformat->name, (pFormatCtx->duration) / 1000000,
            pCodecCtx->width, pCodecCtx->height, pCodec->name);


    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
//
    avcodec_close(pCodecCtx);
    avformat_free_context(pFormatCtx);

    return (*env)->NewStringUTF(env, result);

}


JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_playSimpleVideo
        (JNIEnv *env, jobject jthiz, jstring input_jstr, jobject surface) {
    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    //1.注册组件
    av_register_all();
    //封装上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOG_E_DEBUG("%s", "打开输入视频文件失败");
        return;
    }

    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOG_E_DEBUG("%s", "获取视频信息失败");
        return;
    }

    //AVStream包含视频流，音频流，字幕流等等
    //视频解码，需要找到视频流对应的AVStream所在pFormatCtx->streams的索引位置
    int av_stream_video_index = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        //根据类型判断是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            av_stream_video_index = i;
            break;
        }
    }

    //没有找到
    if (av_stream_video_index == -1) {
        LOG_E_DEBUG("%s", "没有找到对应的视频流");
        return;
    }

    //AVCodecContext:编解码上下文
    //4.解码---》》需要解码器
    AVCodecContext *pCodecCtx = pFormatCtx->streams[av_stream_video_index]->codec;
    //编解码上下文-->获得对应的解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOG_E_DEBUG("%s", "找不到对应的解码器");
        return;
    }

    //5.打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOG_E_DEBUG("%s", "解码器有问题");
        return;
    }


    //输出视频信息
    LOG_I_DEBUG("视频的文件格式：%s", pFormatCtx->iformat->name);
    LOG_I_DEBUG("视频时长：%d", (pFormatCtx->duration) / 1000000);
    LOG_I_DEBUG("视频的宽高：%d,%d", pCodecCtx->width, pCodecCtx->height);
    LOG_I_DEBUG("解码器的名称：%s", pCodec->name);


    //native原生绘制
    //窗体
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;
    //设置缓冲区属性（宽高，像素格式）
    ANativeWindow_setBuffersGeometry(nativeWindow, pCodecCtx->width, pCodecCtx->height,
                                     WINDOW_FORMAT_RGBA_8888);




//    AVPacket avPacket;
//    av_init_packet(&avPacket);
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //像素数据（解码数据）
    AVFrame *yuv_frame = av_frame_alloc();
//    AVFrame *sca_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();


    //********************************************************************************
    //用于像素格式转换或者缩放
//    struct SwsContext *sws_ctx = sws_getContext(
//            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
//            pCodecCtx->width / 2, pCodecCtx->height / 2, AV_PIX_FMT_RGB24,
//            SWS_BILINEAR, NULL, NULL, NULL);
    //********************************************************************************

    //6.一帧一帧读取压缩的视频数据存到编码数据AVPacket
    int len, got_frame, frame_count = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码视频类型的Packet
        if (packet->stream_index == av_stream_video_index) {


            //解码AVpacket->AVFrame
            len = avcodec_decode_video2(pCodecCtx, yuv_frame, &got_frame, packet);
            if (got_frame) {
                LOG_E_DEBUG("解码%d帧", frame_count++);
                //lock
                ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

                //设置rgb_frame的属性（像素格式，宽高）和缓冲区
                //rgb_frame缓冲区与outBuffer.bits是同一块内存，只要rgb_frame缓冲区改变，就可以改变outBuffer
                avpicture_fill((AVPicture *) rgb_frame, outBuffer.bits, PIX_FMT_RGBA,
                               pCodecCtx->width, pCodecCtx->height);

                //**********************************************************************
                //转为指定的YUV420P像素帧
//            sws_scale(sws_ctx,
//                      sca_frame->data,sca_frame->linesize, 0, sca_frame->height,
//                      yuv_frame->data, yuv_frame->linesize);
                //**********************************************************************

                //YUV->RGBA_8888
                I420ToABGR(yuv_frame->data[0], yuv_frame->linesize[0],
                           yuv_frame->data[1], yuv_frame->linesize[1],
                           yuv_frame->data[2], yuv_frame->linesize[2],
                           rgb_frame->data[0], rgb_frame->linesize[0],
                           pCodecCtx->width, pCodecCtx->height);

                //unlock
                ANativeWindow_unlockAndPost(nativeWindow);
                usleep(1000 * 16);
            }
            av_free_packet(packet);
        }
    }
    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    avcodec_close(pCodecCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
}


JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_playSimpleAudio
        (JNIEnv *env, jobject jthiz, jstring input_jstr, jstring output_jstr) {
    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    const char *output_cstr = (*env)->GetStringUTFChars(env, output_jstr, NULL);

    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOG_E_DEBUG("%s", "无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOG_E_DEBUG("%s", "无法获取输入文件信息");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_index = -1;
    for (; pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }

    if (audio_stream_index == -1) {
        LOG_E_DEBUG("%s", "没有找到对应的视频流");
        return;
    }

    //获取解码器
    AVCodecContext *codecContext = pFormatCtx->streams[audio_stream_index]->codec;
    AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
    if (codec == NULL) {
        LOG_E_DEBUG("%s", "找不到对应的音频解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        LOG_E_DEBUG("%s", "无法打开解码器");
        return;
    }

    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //转化 frame->16nit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrContext = swr_alloc();
    //设置重采样参数-----------------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecContext->sample_fmt;
    //输出的采样格式
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecContext->sample_rate;
    //输出采样率
    int out_sample_rate = in_sample_rate;

    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stesreo）
    uint64_t in_ch_layout = codecContext->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    swr_init(swrContext);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
    //设置重采样参数-----------------------end


    //JNI begin------------------
    jclass player_class = (*env)->GetObjectClass(env, jthiz);
    //AudioTrack对象
    jmethodID create_audio_track_mid = (*env)->GetMethodID(env, player_class, "createAudioTrack",
                                                           "(II)Landroid/media/AudioTrack;");
    jobject audio_track = (*env)->CallObjectMethod(env, jthiz, create_audio_track_mid,
                                                   out_sample_rate, out_channel_nb);

    //调用AudioTrack.play方法
    jclass audio_track_class = (*env)->GetObjectClass(env, audio_track);
    jmethodID audio_track_play_mid = (*env)->GetMethodID(env, audio_track_class, "play", "()V");
    (*env)->CallVoidMethod(env, audio_track, audio_track_play_mid);

    //AudioTrack.write
    jmethodID audio_track_write_mid = (*env)->GetMethodID(env, audio_track_class, "write",
                                                          "([BII)I");
    //JNI end------------------
    FILE *fp_pcm = fopen(output_cstr, "wb");

    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, index = 0, ret;
    //不断读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码音频类型的Packet
        if (packet->stream_index == audio_stream_index) {
            //解码
            ret = avcodec_decode_audio4(codecContext, frame, &got_frame, packet);
            if (ret < 0) {
                LOG_E_DEBUG("%s", "解码完成");
            }
            //解码一帧成功
            if (got_frame > 0) {
                LOG_E_DEBUG("解码：%d", index++);
                swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) frame->data, frame->nb_samples);
                //获取sample的size
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 frame->nb_samples, out_sample_fmt,
                                                                 1);
                fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
                //AudioTrack.write 参数
                //out_buffer缓冲区数据，转成byte数组
                jbyteArray audio_sample_array = (*env)->NewByteArray(env, out_buffer_size);
                //得到操作audio_sample_array数组的指针
                jbyte *sample_bytep = (*env)->GetByteArrayElements(env, audio_sample_array, NULL);
                //out_buffer的数据复制到sampe_bytep
                memcpy(sample_bytep, out_buffer, out_buffer_size);
                //同步
                (*env)->ReleaseByteArrayElements(env, audio_sample_array, sample_bytep, 0);
                //AudioTrack.write PCM数据
                (*env)->CallIntMethod(env, audio_track, audio_track_write_mid, audio_sample_array,
                                      0, out_buffer_size);
                //释放局部引用
                (*env)->DeleteLocalRef(env, audio_sample_array);
                usleep(1000 * 16);
            }
            av_free_packet(packet);
        }
    }

    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&pFormatCtx);
    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
    (*env)->ReleaseStringUTFChars(env, output_jstr, output_cstr);
}

//OpenSl ES (Open Sound Library embed system)
JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_playSimpleAudioForOpensl
        (JNIEnv *env, jobject jthiz, jstring input_jstr, jstring output_jstr){}
//======================多线程测试====================================
//JavaVM 代表的是Java虚拟机，所有的工作都是从JavaVM开始
//可以通过JavaVM获取到每个线程关联的JNIEnv
//如何获取JavaVM？
//1.在JNI_OnLoad函数中获取
//2.(*env)->GetJavaVM(env,&javaVM);

//动态库加载时会执行
//兼容Android SDK 2.2之后，2.2没有这个函数
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOG_I_DEBUG("%s", "JNI_OnLoad");
    javaVM = vm;
    return JNI_VERSION_1_4;
}

void *th_fun(void *arg) {
    LOG_I_DEBUG("thread in-->>%s", "th_fun");
    //获取JavaVM
    //通过JavaVM关联当前线程，获取当前线程的JNIEnv
    //(*javaVM)->AttachCurrentThread(javaVM,&env,NULL);
    //(*javaVM)->GetEnv(javaVM,);
    JNIEnv *env = NULL;
    (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
    if (env == NULL) {
        LOG_E_DEBUG("ERROE-->>%s", "env");
        goto end;
    }
    if (uuidutils_class_global == NULL) {
        LOG_E_DEBUG("ERROE-->>%s", "utils_jcls");
        goto end;
    }
    if (uuidutils_get_mid == NULL) {
        LOG_E_DEBUG("ERROE-->>%s", "utils_getuuid_mid");
        goto end;
    }
    char *no = (char *) arg;
    int i = 0;
    for (i = 0; i < 10; i++) {
        LOG_I_DEBUG("thread %s, i:%d", no, i);
        jobject uuid_jstr = (*env)->CallStaticObjectMethod(env, uuidutils_class_global,
                                                           uuidutils_get_mid);
        char *uuid_cstr = (*env)->GetStringUTFChars(env, uuid_jstr, NULL);
        LOG_I_DEBUG("从java获取的UUID是：%s", uuid_cstr);
        (*env)->ReleaseStringUTFChars(env, uuid_jstr, uuid_cstr);
        sleep(1);
    }
    end:
    //解除关联
    (*javaVM)->DetachCurrentThread(javaVM);
    pthread_exit((void *) 0);
}

JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_initThread
        (JNIEnv *env, jobject jthiz) {
    LOG_E_DEBUG("initThread-->>%s", "start");
    //获取class必须要在主线程中,在子线程中是找不到的，偶尔会有。但是找到java.lang.string是可以找到的
    jclass uuidutils_class_tmp = (*env)->FindClass(env, "com/wzf/ffmpeg/Utils");
    if (uuidutils_class_tmp == NULL) {
        LOG_E_DEBUG("ERROE -->>%s", "uuidutils_class_global");
        return;
    } else {
        LOG_E_DEBUG("success -->>%s", "uuidutils_class_global");
    }
    //创建全局引用
    uuidutils_class_global = (*env)->NewGlobalRef(env, uuidutils_class_tmp);
    //获取jmethodId也可以在子线程中
    uuidutils_get_mid = (*env)->GetStaticMethodID(env, uuidutils_class_global, "getUUid",
                                                  "()Ljava/lang/String;");
    if (uuidutils_get_mid == NULL) {
        LOG_E_DEBUG("ERROE -->>%s", "uuidutils_get_mid");
        return;
    }

    LOG_E_DEBUG("success -->>%s", "uuidutils_get_mid");
}


JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_threadTest
        (JNIEnv *env, jobject jthiz) {
    //创建多线程
    LOG_E_DEBUG("thread-->>%s", "start");
    pthread_t tid;
    pthread_create(&tid, NULL, th_fun, "NO1");
}

JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_destroyThread
        (JNIEnv *env, jobject jthiz) {
    //释放全局引用
    (*env)->DeleteGlobalRef(env, uuidutils_class_global);
}
//======================多线程测试====================================
