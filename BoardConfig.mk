TARGET_ARCH_VARIANT := x86-atom
TARGET_HAS_THIRD_PARTY_APPS := true
BOARD_USES_TSLIB := false
BOARD_GPU_DRIVERS := i915
BOARD_USES_KBDSENSOR := false
USE_CAMERA_STUB := false
BOARD_CAMERA_LIBRARIES := libcamera
BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO := true
BUILD_WITH_ALSA_UTILS := true

include $(GENERIC_X86_CONFIG_MK)
