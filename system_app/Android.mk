#
# Copyright (C) 2011 The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#

LOCAL_PATH := $(call my-dir)
LOCAL_APPS := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/*$(COMMON_ANDROID_PACKAGE_SUFFIX)))

define include-app
include $(CLEAR_VARS)

LOCAL_MODULE := $$(basename $(1))
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := $$(suffix $(1))
LOCAL_SRC_FILES := $(1)
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := PRESIGNED
include $$(BUILD_PREBUILT)

ALL_DEFAULT_INSTALLED_MODULES += $$(LOCAL_INSTALLED_MODULE)
endef

$(foreach a,$(LOCAL_APPS),$(eval $(call include-app,$(a))))
