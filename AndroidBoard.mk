LOCAL_PATH := $(call my-dir)
LOCAL_FIRMWARES := ath3k-1.fw $(notdir $(wildcard device/common/firmware/iwlwifi-*))
TARGET_INITRD_SCRIPTS := $(LOCAL_PATH)/wetab_info
TARGET_KERNEL_CONFIG := $(LOCAL_PATH)/wetab_defconfig
TARGET_PREBUILT_APPS := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/system_app/*))
$(call add-prebuilt-targets,$(TARGET_OUT),$(TARGET_PREBUILT_APPS))
TARGET_EXTRA_KERNEL_MODULES := ath3k btusb asus-laptop hid-multitouch
TARGET_EXTRA_KERNEL_MODULES += crystalhd
include $(GENERIC_X86_ANDROID_MK)
