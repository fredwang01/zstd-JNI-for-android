#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define CONSOLE_PRINT 0
#define LOGCAT_PRINT  1

static const char *TAG = "ZSTD";

#if (LOGCAT_PRINT > 0)
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG,__VA_ARGS__) 
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG,__VA_ARGS__) 
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG,__VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)
#elif (CONSOLE_PRINT > 0)	
#define LOGV(...) printf(__VA_ARGS__) 
#define LOGD(...) printf(__VA_ARGS__)  
#define LOGI(...) printf(__VA_ARGS__)  
#define LOGW(...) printf(__VA_ARGS__)  
#define LOGE(...) printf(__VA_ARGS__) 
#else
#define LOGV(...) do{}while(0)
#define LOGD(...) do{}while(0)
#define LOGI(...) do{}while(0)
#define LOGW(...) do{}while(0)  
#define LOGE(...) do{}while(0)
#endif 


#ifdef __cplusplus
}
#endif

#endif 


