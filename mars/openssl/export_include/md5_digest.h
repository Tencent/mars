//
//  md5_digest.h
//  PublicComponent
//
//  Created by liucan on 13-7-22.
//  Copyright (c) 2013 tencent. All rights reserved.
//

#ifndef PublicComponent_md5_digest_h
#define PublicComponent_md5_digest_h

#ifdef __cplusplus
extern "C" {
#endif

struct MD5_CTX;
    
void md5_digest(const char* inBuffer, int len, unsigned char outBuf[16]);
void md5_sig16_to_hex_string(const char* sig, char* outBuf, unsigned int outBufLen);

//fd user for distinct from old interface
int md5_init_fd(MD5_CTX **c);
int md5_update_fd(MD5_CTX *c, const void *data, size_t len);
int md5_final_fd(MD5_CTX *c, unsigned char outBuf[16]);
int md5_uninit_fd(MD5_CTX *c);
    
#ifdef __cplusplus
}
#endif

#endif
