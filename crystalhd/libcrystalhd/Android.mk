LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libcrystalhd
LOCAL_MODULE_TAGS := eng

LOCAL_CFLAGS :=  -D__LINUX_USER__
LOCAL_CFLAGS +=  -O2 -Wall -fPIC -fstrict-aliasing -msse2
#some debugging
LOCAL_CFLAGS +=  -DLOG_NDEBUG=0 

LOCAL_SRC_FILES :=  libcrystalhd_if.cpp \
	libcrystalhd_int_if.cpp \
	libcrystalhd_fwcmds.cpp \
	libcrystalhd_priv.cpp \
	libcrystalhd_fwdiag_if.cpp \
	libcrystalhd_fwload_if.cpp \
	libcrystalhd_parser.cpp \
	fixes.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/link
LOCAL_SHARED_LIBRARIES:= libutils liblog

include $(BUILD_SHARED_LIBRARY)
