TARGET_ARCH_VARIANT := x86-atom
TARGET_HAS_THIRD_PARTY_APPS := true
BOARD_USES_TSLIB := false
BOARD_GPU_DRIVERS := i915
BOARD_USES_KBDSENSOR := true
include $(GENERIC_X86_CONFIG_MK)
BOARD_KERNEL_CMDLINE := root=/dev/ram0 i915.i915_enable_rc6=1 i915.i915_enable_fbc=1 i915.lvds_downclock=1 i915.modeset=1 ehci_force_handoff=1 pcie_aspm=force  androidboot.hardware=wetab
