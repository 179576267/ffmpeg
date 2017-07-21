
#ifndef NDKSTUDY_LOG_H
#define NDKSTUDY_LOG_H
#include <android/log.h>
#define DEFAULT_LOG_TAG  "LOG_FROM_C"
#define LOG_I(TAG,FORMAT,...) __android_log_print(ANDROID_LOG_INFO,  TAG, FORMAT, __VA_ARGS__);
#define LOG_E(TAG,FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, TAG, FORMAT, __VA_ARGS__);
#define LOG_W(TAG,FORMAT,...) __android_log_print(ANDROID_LOG_WARN,  TAG, FORMAT, __VA_ARGS__);
#define LOG_D(TAG,FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG, TAG, FORMAT, __VA_ARGS__);

#define LOG_I_DEBUG(FORMAT,...) LOG_I(DEFAULT_LOG_TAG, FORMAT, __VA_ARGS__);
#define LOG_E_DEBUG(FORMAT,...) LOG_E(DEFAULT_LOG_TAG, FORMAT, __VA_ARGS__);
#define LOG_W_DEBUG(FORMAT,...) LOG_W(DEFAULT_LOG_TAG, FORMAT, __VA_ARGS__);
#define LOG_D_DEBUG(FORMAT,...) LOG_D(DEFAULT_LOG_TAG, FORMAT, __VA_ARGS__);
#endif //NDKSTUDY_LOG_H
