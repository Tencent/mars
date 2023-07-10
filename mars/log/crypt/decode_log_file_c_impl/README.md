### 因为增加了 zstd 的支持，所以不再提供 cmake 之外的编译方式

编译之前记得先在 mars 仓库中执行`git submodule update --init`



1. 替换 decode_log_file.c 中的 PRIV_KEY 和 PUB_KEY
2. 编译

```
mkdir -p cmake_build && cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j
```
产物是 cmake_build/decode_log_file

