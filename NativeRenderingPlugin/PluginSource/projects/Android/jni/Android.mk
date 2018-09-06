LOCAL_PATH := $(call my-dir)
SRC_DIR := ../../../source

include $(CLEAR_VARS)
LOCAL_MODULE := RenderingPlugin
LOCAL_C_INCLUDES += $(SRC_DIR)
LOCAL_LDLIBS := -llog
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/RenderingPlugin.cpp

# OpenGL ES
LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_OpenGLCoreES.cpp
LOCAL_LDLIBS += -lGLESv2
LOCAL_CPPFLAGS += -DSUPPORT_OPENGL_ES=1

# Vulkan (optional)
LOCAL_SRC_FILES += $(SRC_DIR)/RenderAPI_Vulkan.cpp
LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/third_party/vulkan/src/include
LOCAL_CPPFLAGS += -DSUPPORT_VULKAN=1

# build
include $(BUILD_SHARED_LIBRARY)
