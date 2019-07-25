#!/usr/bin/env sh
cmake -DANDROID_ABI="armeabi" -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE=/Applications/sdks/android-ndk-r16b/build/cmake/android.toolchain.cmake  -DANDROID_TOOLCHAIN=clang -DANDROID_NDK=/Applications/sdks/android-ndk-r16b -DANDROID_PLATFORM=android-14 -DANDROID_STL="c++_shared" && make -j8 && make install
