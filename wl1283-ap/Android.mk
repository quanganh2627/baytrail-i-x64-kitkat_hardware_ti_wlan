ifeq ($(BOARD_SOFTAP_DEVICE),$(BOARD_WLAN_DEVICE)-ap)
    include $(call all-subdir-makefiles)
endif
