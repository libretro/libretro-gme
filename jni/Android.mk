LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DIR := ../../src

LOCAL_MODULE    := retro

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DANDROID_ARM
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CFLAGS += -DANDROID_MIPS -D__mips__ -D__MIPSEL__
endif

CORE_DIR := ..

include $(CORE_DIR)/Makefile.common

LOCAL_SRC_FILES    += $(SOURCES_C) $(SOURCES_CXX)
LOCAL_CFLAGS 		 += -O3 -std=gnu99 -ffast-math -funroll-loops -DBLARGG_LITTLE_ENDIAN -DIOAPI_NO_64
LOCAL_C_INCLUDES   = $(CORE_DIR)/deps/libretro-common/include \
							$(CORE_DIR)/deps/game-music-emu/gme \
							$(CORE_DIR)/deps/zlib-1.2.8 \
							$(CORE_DIR)/deps/zlib-1.2.8/contrib/minizip


include $(BUILD_SHARED_LIBRARY)
