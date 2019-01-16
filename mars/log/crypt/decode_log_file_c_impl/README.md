1. 修改 decode_log_file.c 中的 PRIV_KEY 和 PUB_KEY
2. 使用 cmake 编译
服务器上没装 cmake 的话，用 gcc 编译：

```
gcc decode_log_file.c micro-ecc-master/uECC.c -o decode_log_file -O0 -ggdb -lz
```
