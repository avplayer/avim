APP_OPTIM := release  
APP_STL               := gnustl_static
APP_ABI               := all
#APP_ABI              := all armeabi-v7a mips x86 mips64 x86_64
APP_PROJECT_PATH 	  := $(call my-dir)
APP_BUILD_SCRIPT      := $(call my-dir)/Android.mk
APP_MODULES           := libprotobuf-cpp-lite libprotobuf-cpp-full
