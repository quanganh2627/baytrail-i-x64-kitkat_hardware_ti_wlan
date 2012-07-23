# This makefile is included from vendor/intel/*/AndroidBoard.mk.

REGDOM=$(word 2,$(subst _, ,$(word 1,$(PRODUCT_LOCALES))))

.PHONY: build_wl12xx-compat
build_wl12xx-compat: build_kernel
	TARGET_TOOLS_PREFIX="$(ANDROID_BUILD_TOP)/$(TARGET_TOOLS_PREFIX)" vendor/intel/support/wl12xx-compat-build.sh -c $(CUSTOM_BOARD) -r $(REGDOM)

