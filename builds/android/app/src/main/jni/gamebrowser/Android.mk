LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gamebrowser

EASYRPG_TOOLCHAIN_DIR = $(EASYDEV_ANDROID)/$(TARGET_ARCH_ABI)-toolchain

LOCAL_C_INCLUDES := \
	$(EASYRPG_TOOLCHAIN_DIR)/include \
	$(EASYRPG_TOOLCHAIN_DIR)/include/libpng16

LOCAL_SRC_FILES := \
	org_easyrpg_player_game_browser_GameScanner.cpp

LOCAL_LDLIBS := -L$(EASYRPG_TOOLCHAIN_DIR)/lib -llog -lz -lpng

LOCAL_CFLAGS := -O2 -Wall -Wextra -fno-rtti

LOCAL_CPPFLAGS = $(LOCAL_C_FLAGS) -std=c++11

include $(BUILD_SHARED_LIBRARY)
