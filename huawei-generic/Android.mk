# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    huaweigeneric-ril.c \
    atchannel.c \
    misc.c \
    at_tok.c \
    sms.c \
    sms_gsm.c \
    gsm.c \
	requestdatahandler.c \
	fcp_parser.c \
	audiochannel.cpp 
	

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libril libmedia

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_C_INCLUDES := $(KERNEL_HEADERS) $(TOP)/hardware/ril/libril/

LOCAL_MODULE_TAGS := optional

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
	libcutils libutils
  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB 
  LOCAL_MODULE:= libhuaweigeneric-ril
  LOCAL_PRELINK_MODULE := false
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
	libril
  LOCAL_MODULE:= huaweigeneric-ril
  include $(BUILD_EXECUTABLE)
endif 