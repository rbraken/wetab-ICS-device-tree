PRODUCT_PACKAGES := $(THIRD_PARTY_APPS)
PRODUCT_PACKAGES += sensors.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += gps.$(TARGET_PRODUCT)
PRODUCT_PACKAGES += libhuaweigeneric-ril
PRODUCT_PACKAGES += wetab_powerbtnd
PRODUCT_PACKAGES += pppd
PRODUCT_PACKAGES += Bluetooth Superuser Development android-common-carousel

PRODUCT_COPY_FILES += $(shell find $(LOCAL_PATH)/system_overlay -type f | \
  $(SED_EXTENDED) "s:($(LOCAL_PATH)/system_overlay/?(.*)):\\1\\:system/\\2:" )

PRODUCT_PROPERTY_OVERRIDES :=  poweroff.doubleclick=1

$(call inherit-product,$(SRC_TARGET_DIR)/product/generic_x86.mk)

PRODUCT_NAME := wetab
PRODUCT_DEVICE := wetab
PRODUCT_MANUFACTURER := Pegatron

PRODUCT_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlays
