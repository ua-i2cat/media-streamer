# Copyright (C) 2010 The Android Open Source Project
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
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ug-test
LOCAL_SRC_FILES := main.c
LOCAL_LDLIBS    := -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src

LOCAL_CFLAGS += -std=c99 #$(LOCAL_PATH)/../../android-ndk-r8e/platforms/android-14/arch-arm/usr/include  $(LOCAL_PATH)/../../android-ndk-r8e/platforms/android-14/arch-arm/usr/lib

LOCAL_LDLIBS +=  #-lrt -lpthread -lieee -lm -pthread #$(LOCAL_PATH)/../../android-ndk-r8e/platforms/android-14/arch-arm/usr/lib 

LOCAL_SRC_FILES += 	src/debug.c \
					src/host.c \
					src/perf.c \
					src/ntp.c \
					src/pdb.c \
					src/tile.c \
					src/tv.c \
					src/transmit.c \
					src/tfrc.c \
					src/rtp/pbuf.c \
					src/rtp/decoders.c \
					src/rtp/audio_decoders.c \
					src/rtp/ptime.c \
					src/rtp/net_udp.c \
					src/rtp/ll.cpp \
					src/rtp/rtp.c \
					src/rtp/rtp_callback.c \
					src/audio/audio.c \
					src/audio/audio_capture.c \
					src/audio/audio_playback.c \
					src/audio/capture/none.c \
					src/audio/codec.c \
					src/audio/codec/dummy_pcm.c \
					src/audio/export.c \
					src/audio/playback/none.c \
					src/audio/resampler.c \
					src/audio/utils.c \
					src/compat/drand48.c \
					src/compat/gettimeofday.c \
					src/compat/inet_ntop.c \
					src/compat/inet_pton.c \
					src/compat/platform_semaphore.c \
					src/compat/platform_time.c \
					src/compat/usleep.c \
					src/crypto/crypt_aes.c \
					src/crypto/crypt_aes_impl.c \
					src/crypto/crypt_des.c \
					src/crypto/md5.c \
					src/crypto/random.c \
					src/utils/list.c \
					src/utils/packet_counter.cpp \
					src/utils/resource_manager.cpp \
					src/utils/ring_buffer.c \
					src/utils/worker.cpp \
					src/video.c \
					src/video_codec.c \
					src/video_capture.c \
					src/video_capture/null.c \
					src/video_capture/net.c \
					src/video_compress.c \
					src/video_compress/none.c \
					src/video_decompress.c \
					src/video_decompress/null.c \
					src/video_display.c \
					src/video_display/null.c \
					src/video_display/net.c \
					src/video_export.c \
					src/main.c \

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)