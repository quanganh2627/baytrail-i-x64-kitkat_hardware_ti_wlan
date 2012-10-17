# This makefile is included from vendor/intel/common/wifi/WifiRules.mk.

.PHONY: build_wl12xx-compat
build_wl12xx-compat: build_kernel
	TARGET_TOOLS_PREFIX="$(ANDROID_BUILD_TOP)/$(TARGET_TOOLS_PREFIX)" vendor/intel/support/wl12xx-compat-build.sh -c $(TARGET_DEVICE)

$(PRODUCT_OUT)/ramdisk.img : build_wl12xx-compat
