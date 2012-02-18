PRODUCT_PACKAGES := $(THIRD_PARTY_APPS)
PRODUCT_PACKAGES += sensors.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += gps.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += lights.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += audio.primary.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += audio_policy.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += Camera
PRODUCT_PACKAGES += pppd
#PRODUCT_PACKAGES += libtinyalsa
#PRODUCT_PACKAGES += tinyplay tinycap tinymix
PRODUCT_PACKAGES += Bluetooth Superuser CameraPreviewTest procstatlog 

PRODUCT_COPY_FILES := \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    device/common/generic_x86/GenericTouch.idc:system/usr/idc/Vendor_0eef_Product_72a1.idc \
    $(LOCAL_PATH)/vold.fstab:system/etc/vold.fstab \
    device/common/firmware/ath3k-1.fw:system/lib/firmware/ath3k-2.fw \
    $(LOCAL_PATH)/gps.conf:system/etc/gps.conf \
    $(LOCAL_PATH)/crystalhd/firmware/fwbin/70015/bcm70015fw.bin:system/lib/firmware

$(call inherit-product,$(SRC_TARGET_DIR)/product/generic_x86.mk)

PRODUCT_NAME := wetab
PRODUCT_DEVICE := wetab
PRODUCT_MANUFACTURER := Pegatron

DEVICE_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlays
