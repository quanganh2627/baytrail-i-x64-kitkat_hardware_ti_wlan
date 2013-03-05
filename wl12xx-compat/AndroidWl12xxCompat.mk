# This makefile is included from vendor/intel/common/wifi/WifiRules.mk.

.PHONY: build_wl12xx-compat
build_wl12xx-compat: build_kernel
	+TARGET_TOOLS_PREFIX="$(ANDROID_BUILD_TOP)/$(TARGET_TOOLS_PREFIX)" \
	TARGET_DEVICE="$(TARGET_DEVICE)" \
	TARGET_BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
	KERNEL_SRC_DIR="$(KERNEL_SRC_DIR)" \
	PRODUCT_OUT="`pwd`/$(PRODUCT_OUT)" \
	vendor/intel/support/kernel-build.sh \
	-X hardware/ti/wlan/wl12xx-compat \
	-f CFLAGS_main.o=-I$(ANDROID_BUILD_TOP)/hardware/intel/PRIVATE/monitor/inc

$(PRODUCT_OUT)/ramdisk.img : build_wl12xx-compat
