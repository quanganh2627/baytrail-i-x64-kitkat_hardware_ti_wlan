# This makefile is included from vendor/intel/*/AndroidBoard.mk.

.PHONY: build_wl1283
build_wl1283: build_kernel
	TARGET_TOOLS_PREFIX="$(ANDROID_BUILD_TOP)/$(TARGET_TOOLS_PREFIX)" vendor/intel/support/kernel-build.sh -c $(CUSTOM_BOARD) -M hardware/ti/wlan/wl1283
