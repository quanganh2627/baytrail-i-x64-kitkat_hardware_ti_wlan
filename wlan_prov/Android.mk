LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CHAABI_ROOT = hardware/intel/PRIVATE/chaabi


ifeq ($(BUILD_WITH_CHAABI_SUPPORT),true)
LOCAL_C_INCLUDES = \
	$(TARGET_OUT_HEADERS)/chaabi
endif

LOCAL_SRC_FILES:= \
	wlan_provisioning.c

LOCAL_CFLAGS:=
LOCAL_CFLAGS += -DNLCP

ifeq ($(BUILD_WITH_CHAABI_SUPPORT),true)
LOCAL_CFLAGS += -DBUILD_WITH_CHAABI_SUPPORT
LOCAL_STATIC_LIBRARIES := \
	CC6_UMIP_ACCESS CC6_ALL_BASIC_LIB
endif


LOCAL_SHARED_LIBRARIES := \
	libc libcutils libhardware_legacy

LOCAL_MODULE:= wlan_prov
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

