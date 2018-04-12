# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Bug-77486542
LOCAL_SRC_FILES := poc.cpp
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)32
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)64

LOCAL_C_INCLUDES := \
    $(TOP)/frameworks/av/include/media/ \
    $(TOP)/frameworks/native/include/media/openmax \
    $(TOP)/frameworks/native/include/media/hardware \
    $(TOP)/frameworks/av/media/libstagefright/omx \
    $(TOP)/frameworks/av/media/libstagefright/

LOCAL_SHARED_LIBRARIES := \
    libstagefright \
    libstagefright_omx \
    libstagefright_foundation \
    libhidlbase \
    android.hidl.memory@1.0 \
    android.hidl.token@1.0-utils \
    android.hardware.media.omx@1.0 \
    android.hardware.media@1.0\
    android.hidl.allocator@1.0 \
    android.hidl.memory@1.0 \
    libhidlmemory \
    libutils \
    libmedia \
    liblog \
    libbinder\
    libcrypto\
    libcutils \
    libui

LOCAL_COMPATIBILITY_SUITE := sts
LOCAL_CTS_TEST_PACKAGE := android.security.sts

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -Wall -Werror

include $(BUILD_CTS_EXECUTABLE)
