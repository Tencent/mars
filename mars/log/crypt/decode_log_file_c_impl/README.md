服务器上没装 cmake 的话，用 gcc 编译：

```
gcc decode_log_file.c micro-ecc-master/uECC.c -o decode_log_file -O0 -ggdb -lz
```
