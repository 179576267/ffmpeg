#include "header/com_wzf_ffmpeg_VideoUtils.h"
#include<stdio.h>
#include "./header/android_log.h"

//编码
#include "header/include/libavcodec/avcodec.h"
//封装格式处理
#include "header/include/libavformat/avformat.h"
//像素处理
#include "header/include/libswscale/swscale.h"


JNIEXPORT void JNICALL Java_com_wzf_ffmpeg_VideoUtils_decode(JNIEnv *env, jclass jcls,
                                                             jstring input_jstr,
                                                             jstring output_jstr) {
    const char* input_cstr = (*env)->GetStringUTFChars(env,input_jstr,NULL);
    const char* output_cstr = (*env)->GetStringUTFChars(env,output_jstr,NULL);

    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
        LOG_I_DEBUG("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOG_I_DEBUG("%s","获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for(; i < pFormatCtx->nb_streams;i++){
        //根据类型判断，是否是视频流
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }

    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if(pCodec == NULL){
        LOG_I_DEBUG("%s","无法解码");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodeCtx,pCodec,NULL) < 0){
        LOG_I_DEBUG("%s","解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height);


    //输出文件
    FILE* fp_yuv = fopen(output_cstr,"wb");

    //用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            pCodeCtx->width, pCodeCtx->height, pCodeCtx->pix_fmt,
            pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    int len ,got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if(got_frame){
            //frame->yuvFrame (YUV420P)
            //转为指定的YUV420P像素帧
            sws_scale(sws_ctx,
                      frame->data,frame->linesize, 0, frame->height,
                      yuvFrame->data, yuvFrame->linesize);

            //向YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size = pCodeCtx->width * pCodeCtx->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);

            LOG_I_DEBUG("解码%d帧",framecount++);
        }

        av_free_packet(packet);
    }

    fclose(fp_yuv);

    av_frame_free(&frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env,input_jstr,input_cstr);
    (*env)->ReleaseStringUTFChars(env,output_jstr,output_cstr);
}


jstring JNICALL Java_com_wzf_ffmpeg_VideoUtils_getVideoInfo (JNIEnv *env , jclass jcls, jstring input_jstr){
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
    sprintf(result, "视频的文件格式：%s, \n视频时长：%lld, \n视频的宽高：%d,%d, \n 解码器的名称：%s", pFormatCtx->iformat->name, (pFormatCtx->duration) / 1000000,
    pCodecCtx->width, pCodecCtx->height, pCodec->name);


    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
//
    avcodec_close(pCodecCtx);
    avformat_free_context(pFormatCtx);

    return (*env)->NewStringUTF(env, result);

}
