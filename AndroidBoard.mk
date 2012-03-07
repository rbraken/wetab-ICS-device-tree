LOCAL_PATH := $(call my-dir)
TARGET_INITRD_SCRIPTS := $(LOCAL_PATH)/wetab_info
TARGET_KERNEL_CONFIG := $(LOCAL_PATH)/wetab_defconfig
TARGET_EXTRA_KERNEL_MODULES := ath3k btusb hid-multitouch asus-laptop crystalhd

include $(GENERIC_X86_ANDROID_MK)
