# Copyright (C) 2011 The Android-x86 Open Source Project

LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked and stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_MODULE := sensors.$(TARGET_PRODUCT)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := kbdsensor.cpp

LOCAL_CFLAGS := -DFN_ROT_0=KEY_F9 -DFN_ROT_90=KEY_F12 -DFN_ROT_180=KEY_F10 -DFN_ROT_270=KEY_F11

include $(BUILD_SHARED_LIBRARY)
