#include <jni.h>
#include <string.h>
#include <fcntl.h>
#include "wrapper.h"
#if (COMPILE_ZSTD > 0)
#include "zstd.h"
#endif //



///////////////////////////////////////////////////////////////////////////////
// extern 
#if (COMPILE_ZSTD > 0)
extern int getDstreamStage(ZSTD_DStream* zds);
#endif 


static void setErrCode(JNIEnv *env, jobject obj, int errCode) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "errCode", "I");
	(*env)->SetIntField(env, obj, fid, errCode);
}


///////////////////////////////////////////////////////////////////////////////
// zstd stream API
#if (COMPILE_ZSTD > 0)

#define JAVA_ZSTD_CINIT "nt1"
#define JAVA_ZSTD_STREAM_COMPRESS "nt2"
#define JAVA_ZSTD_DINIT "nt3"
#define JAVA_ZSTD_STREAM_DECOMPRESS "nt4"

#define SIG_ZSTD_CINIT "(I)J"
#define SIG_ZSTD_STREAM_COMPRESS "(J[BIII)[B"
#define SIG_ZSTD_DINIT "()J"
#define SIG_ZSTD_STREAM_DECOMPRESS "(J[BIII)[B"

static const char *java_cls_zstd = "com/fred/zstd/Zstd";
jlong JNICALL cinit(JNIEnv *env, jobject obj, jint level);
jbyteArray JNICALL zstd_stream_compress(JNIEnv *env, jobject obj, jlong instance, jbyteArray src, jint offset, jint srcLen, jint isEnd);
jlong JNICALL dinit(JNIEnv *env, jobject obj) ;
jbyteArray JNICALL zstd_stream_decompress(JNIEnv *env, jobject obj, jlong instance, jbyteArray src, jint offset, jint srcLen, jint isEnd);

static JNINativeMethod zstd_methods[] = {
	{JAVA_ZSTD_CINIT, SIG_ZSTD_CINIT, (void *)cinit},
	{JAVA_ZSTD_STREAM_COMPRESS, SIG_ZSTD_STREAM_COMPRESS, (void *)zstd_stream_compress},
	{JAVA_ZSTD_DINIT, SIG_ZSTD_DINIT, (void *)dinit},
	{JAVA_ZSTD_STREAM_DECOMPRESS, SIG_ZSTD_STREAM_DECOMPRESS, (void *)zstd_stream_decompress},
};

struct zstd_compress_instance {
	ZSTD_CStream* cstream;
	void *buffOut;
	size_t buffOutSize;
};

struct zstd_decompress_instance {
	ZSTD_DStream* dstream;
	void *buffOut;
	size_t buffOutSize;
};

jlong JNICALL cinit(JNIEnv *env, jobject obj, jint level) {
	struct zstd_compress_instance *instance = (struct zstd_compress_instance *)malloc(sizeof(struct zstd_compress_instance));
	if (instance == NULL) {
		LOGE("instance create failed.");
		return 0;
	}
	memset(instance, 0, sizeof(struct zstd_compress_instance));
	
	instance->buffOutSize = ZSTD_CStreamOutSize(); 
	instance->buffOut = malloc(instance->buffOutSize);
	if (instance->buffOut == NULL) {
		LOGE("buffer out create failed. size:%ld\n", instance->buffOutSize);
		goto failed;
	}
	instance->cstream = ZSTD_createCStream();
	if (instance->cstream == NULL) {
		LOGE("cstream create failed.");
		goto failed;
	}

	size_t const initResult = ZSTD_initCStream(instance->cstream, level);
	if (ZSTD_isError(initResult)) {
		LOGE("cstream init failed.");
		goto failed;		
	}

	long inst = (long)instance;
        return inst;
	failed: 
	if (instance->buffOut != NULL) {
		free(instance->buffOut);
	}
	if (instance->cstream != NULL) {
		ZSTD_freeCStream(instance->cstream);
	}
	if (instance != NULL) {
		free(instance);
	}
	return 0;
}

jlong JNICALL dinit(JNIEnv *env, jobject obj) {
	struct zstd_decompress_instance *instance = (struct zstd_decompress_instance *)malloc(sizeof(struct zstd_decompress_instance));
	memset(instance, 0, sizeof(struct zstd_decompress_instance));
	if (instance == NULL) {
		LOGE("instance create failed.");
		return 0;
	}
	instance->buffOutSize = ZSTD_DStreamOutSize();  
	instance->buffOut = malloc(instance->buffOutSize);
	if (instance->buffOut == NULL) {
		LOGE("buffer out create failed. size:%ld\n", instance->buffOutSize);
		goto failed;
	}
	instance->dstream = ZSTD_createDStream();	
	if (instance->dstream == NULL) {
		LOGE("dstream create failed.");
		goto failed;
	}

	size_t const initResult = ZSTD_initDStream(instance->dstream);
	if (ZSTD_isError(initResult)) {
		LOGE("dstream init failed.");
		goto failed;		
	}
	
	long inst = (long)instance;
        return inst;
	failed:
	if (instance->buffOut != NULL) {
		free(instance->buffOut);
	}
	if (instance->dstream != NULL) {
		ZSTD_freeDStream(instance->dstream);
	}
	if (instance != NULL) {
		free(instance);
	}
	return 0;
}

static void freeZstdCinstance(struct zstd_compress_instance *pinstance) {
	free(pinstance->buffOut);
	ZSTD_freeCStream(pinstance->cstream);
	free(pinstance);
	pinstance = NULL;
}

static void freeZstdDinstance(struct zstd_decompress_instance *pinstance) {
	free(pinstance->buffOut);
	ZSTD_freeDStream(pinstance->dstream);
	free(pinstance);
	pinstance = NULL;
}

jbyteArray JNICALL zstd_stream_decompress(JNIEnv *env, jobject obj, jlong instance, jbyteArray src, jint offset, jint srcLen, jint isEnd) {
	struct zstd_decompress_instance *pinstance = (struct zstd_decompress_instance *)instance;
	ZSTD_DStream* const dstream = pinstance->dstream;
	void *buffOut = pinstance->buffOut;
	size_t buffOutSize = pinstance->buffOutSize;
	char *buffResult = NULL;
	int needFreeBuffResult = 1;
	int buffResultSize = 0;
	int errCode = 0;
        unsigned char *pSrc = NULL;

	if ((isEnd > 0) && (srcLen == 0)) {
		goto flushFrame;
	}
	pSrc = (unsigned char *)(*env)->GetByteArrayElements(env, src, NULL);
	if (pSrc == NULL) {
		LOGE("get byte array failed.");
		errCode = -1;
		goto out;
	}
	ZSTD_inBuffer input = {pSrc, srcLen, offset};
	int segs = 0;
	int inLoop = -1;
	while (input.pos < input.size) {
		inLoop++;
		ZSTD_outBuffer output = {buffOut, buffOutSize, 0};
		size_t hintSize = ZSTD_decompressStream(dstream, &output, &input);
		if (ZSTD_isError(hintSize)) {
			LOGE("decompress error:%s\n", ZSTD_getErrorName(hintSize));
			errCode = -2;
		        goto out;
		}
		//LOGI("Loop:%d input pos:%d size:%d hitsize:%d output pos:%d dstream stage:%d\n", inLoop, input.pos, input.size, hintSize, output.pos, getStage(dstream));
		if (output.pos > 0) {
			if (segs == 0) {
				buffResult = malloc(output.pos);
				if (buffResult == NULL) {
					LOGE("mem failed.");
		                        errCode = -3;
		                        goto out;
				}
				memcpy(buffResult, buffOut, output.pos);				
			} else {
			       char *curr = malloc(buffResultSize + output.pos);
				if (curr == NULL) {
					LOGE("mem failed.");
		                        errCode = -4;
		                        goto out;
				}
				memcpy(curr, buffResult, buffResultSize);
				memcpy(curr + buffResultSize, buffOut, output.pos);
				free(buffResult);
				buffResult = curr;
			}
			buffResultSize += output.pos;
			segs++;
		}
	}

    flushFrame:
    if (isEnd > 0) {
	// do nothing. only free dstream.
    }

    jbyteArray bytes = (*env)->NewByteArray(env, buffResultSize);
    if (bytes == NULL) {
	LOGE("mem obj failed.");
	errCode = -7;
	goto out;
    }
    if (buffResult != NULL) {
	(*env)->SetByteArrayRegion(env, bytes, 0, buffResultSize, buffResult);
    }

    out:
    // free resource if necessary.
    if (pSrc != NULL) {
        (*env)->ReleaseByteArrayElements(env, src, pSrc, 0);
    }	
    if ((buffResult != NULL) && (needFreeBuffResult > 0)) {
	free(buffResult);
    }
    if (isEnd > 0) {
	freeZstdDinstance(pinstance);
    }
    if (errCode != 0) {
	setErrCode(env, obj, errCode);
	return NULL;
    }	
    return bytes;
}

jbyteArray JNICALL zstd_stream_compress(JNIEnv *env, jobject obj, jlong instance, jbyteArray src, jint offset, jint srcLen, jint isEnd) {
	struct zstd_compress_instance *pinstance = (struct zstd_compress_instance *)instance;
	ZSTD_CStream* const cstream = pinstance->cstream;
	void *buffOut = pinstance->buffOut;
	size_t buffOutSize = pinstance->buffOutSize;
	char *buffResult = NULL;
	int needFreeBuffResult = 1;
	int buffResultSize = 0;
	int errCode = 0;
        unsigned char *pSrc = NULL;

	if ((isEnd > 0) && (srcLen == 0)) {
		goto flushFrame;
	}
	pSrc = (unsigned char *)(*env)->GetByteArrayElements(env, src, NULL);
	if (pSrc == NULL) {
		LOGE("get byte array failed.");
		errCode = -1;
		goto out;
	}
	ZSTD_inBuffer input = { pSrc, srcLen, offset};
	int segs = 0;
	while (input.pos < input.size) {
		ZSTD_outBuffer output = {buffOut, buffOutSize, 0};
		size_t hintSize = ZSTD_compressStream(cstream, &output, &input);
		if (ZSTD_isError(hintSize)) {
			LOGE("compress error:%s\n", ZSTD_getErrorName(hintSize));
			errCode = -2;
		        goto out;
		}
		if (output.pos > 0) {
			if (segs == 0) {
				buffResult = malloc(output.pos);
				if (buffResult == NULL) {
					LOGE("mem failed.");
		                        errCode = -3;
		                        goto out;
				}
				memcpy(buffResult, buffOut, output.pos);				
			} else {
			        char *curr = malloc(buffResultSize + output.pos);
				if (curr == NULL) {
					LOGE("mem failed.");
		                        errCode = -4;
		                        goto out;
				}
				memcpy(curr, buffResult, buffResultSize);
				memcpy(curr + buffResultSize, buffOut, output.pos);
				free(buffResult);
				buffResult = curr;
			}
			buffResultSize += output.pos;
			segs++;
		}
	}

    flushFrame:
    if (isEnd > 0) {
		ZSTD_outBuffer output = {buffOut, buffOutSize, 0};
		size_t const remainingToFlush = ZSTD_endStream(cstream, &output);
		if (remainingToFlush) { 
			LOGE("end cstream failed.");
		        errCode = -5;
		        goto out;
		}
		if (output.pos > 0) {
			if (buffResult == NULL) {
				needFreeBuffResult = 0;
				buffResult = buffOut;				
			} else {
			        char *curr = malloc(buffResultSize + output.pos);
				if (curr == NULL) {
					LOGE("mem failed.");
		                        errCode = -6;
		                        goto out;
				}
				memcpy(curr, buffResult, buffResultSize);
				memcpy(curr + buffResultSize, buffOut, output.pos);
				free(buffResult);
				buffResult = curr;			
			}
			buffResultSize += output.pos;
		}
    }

	jbyteArray bytes = (*env)->NewByteArray(env, buffResultSize);
	if (bytes == NULL) {
		LOGE("mem obj failed.");
		errCode = -7;
		goto out;
	}
	if (buffResult != NULL) {
		(*env)->SetByteArrayRegion(env, bytes, 0, buffResultSize, buffResult);
	}

	out:
        // free resource if necessary.
        if (pSrc != NULL) {
		(*env)->ReleaseByteArrayElements(env, src, pSrc, 0);
        }	
	if ((buffResult != NULL) && (needFreeBuffResult > 0)) {
		free(buffResult);
	}
	if (isEnd > 0) {
		freeZstdCinstance(pinstance);
	}
	if (errCode != 0) {
		setErrCode(env, obj, errCode);
		return NULL;
	}	
	return bytes;
}
#endif // end ZSTD stream API.


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv *env = 0;
	
	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) {
		LOGE("get env failed.");
		return JNI_ERR;
	}

	#if (COMPILE_ZSTD > 0)
	
	jclass clazz = (*env)->FindClass(env, java_cls_zstd);
	if (clazz == NULL) {
		LOGE("NOT find class:%s", java_cls_zstd);
		return JNI_ERR;
	}
	
	if ((*env)->RegisterNatives(env, clazz, zstd_methods, sizeof(zstd_methods)/sizeof(zstd_methods[0])) != JNI_OK) {
		LOGE("register %s error.", java_cls_zstd);
		return JNI_ERR;
	}
	#endif // ZSTD
	
	return JNI_VERSION_1_6;
}


