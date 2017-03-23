LOCAL_PATH := $(call my-dir)
LOCAL_MY_INCLUDES := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE    := zstd
COMMON_SRC := \
    common/entropy_common.c \
	common/fse_decompress.c \
	common/xxhash.c \
	common/zstd_common.c
	
COMPRESS_SRC := \
    compress/fse_compress.c \
	compress/huf_compress.c \
	compress/zbuff_compress.c \
	compress/zstd_compress.c
	
DECOMPRESS_SRC := \
    decompress/huf_decompress.c \
	decompress/zbuff_decompress.c \
	decompress/zstd_decompress.c

DICT_SRC := \
    dictBuilder/divsufsort.c \
	dictBuilder/zdict.c

LOCAL_C_INCLUDES := $(LOCAL_MY_INCLUDES)/common 
ZSTD_SRC = $(COMMON_SRC) $(COMPRESS_SRC) $(DECOMPRESS_SRC) $(DICT_SRC)     
LOCAL_SRC_FILES := $(ZSTD_SRC)  wrapper.c
LOCAL_LDLIBS    := -llog 

# compiled library for android6.0+
LOCAL_LDFLAGS += -pie -fPIE -shared
LOCAL_CFLAGS += -pie -fPIE

include $(BUILD_SHARED_LIBRARY)

