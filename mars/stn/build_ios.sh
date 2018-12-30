cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake \
                -DIOS_PLATFORM=OS       \
                -DENABLE_ARC=0  \
                -DENABLE_BITCODE=0
