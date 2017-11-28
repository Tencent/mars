#!/bin/sh

# build libraries for android
export ANDROID_NDK_HOME=/usr/local/android-ndks/android-ndk-r13b
python mars/librariesbuild_android.py 1 armeabi
