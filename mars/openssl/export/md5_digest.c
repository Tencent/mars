//
//  md5_digest.c
//  PublicComponent
//
//  Created by 刘粲 on 13-7-22.
//  Copyright (c) 2013骞�zhoushaotao. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include "md5_digest.h"

#define HEX_STRING	"0123456789abcdef"

void md5_digest(const char* inBuffer, int len, unsigned char outBuf[16])
{
	MD5_CTX ctx;
    if(NULL == inBuffer) return;
    MD5_Init(&ctx);
	MD5_Update(&ctx, inBuffer, (size_t)len);
	MD5_Final(outBuf, &ctx);
}

void md5_sig16_to_hex_string(const char* sig, char* outBuf, unsigned int outBufLen)
{
    unsigned char	*sig_p;
    char		*str_p, *max_p;
    unsigned int	high, low;
    
    str_p = outBuf;
    max_p = outBuf + outBufLen;
    
    for (sig_p = (unsigned char *)sig;
         sig_p < (unsigned char *)sig + 16;
         sig_p++) {
        high = *sig_p / 16;
        low = *sig_p % 16;
        /* account for 2 chars */
        if (str_p + 1 >= max_p) {
            break;
        }
        *str_p++ = HEX_STRING[high];
        *str_p++ = HEX_STRING[low];
    }
    /* account for 2 chars */
    if (str_p < max_p) {
        *str_p++ = '\0';
    }
}


int md5_init_fd(MD5_CTX **c)
{
	if(NULL == c) return -1;
	*c = malloc(sizeof(MD5_CTX));
	if(NULL == *c) return -2; // no memory
	MD5_Init(*c);
	return 0;
}

int md5_update_fd(MD5_CTX *c, const void *data, size_t len)
{
	if(NULL == c || NULL == data || 0 == len) return -1;
	MD5_Update(c, data, (size_t)len);
	return 0;
}

int md5_final_fd(MD5_CTX *c, unsigned char outBuf[16])
{
	if(NULL == c) return -1;
	MD5_Final(outBuf, c);
	return 0;
}

int md5_uninit_fd(MD5_CTX* c)
{
	if(NULL == c) return -1;
	free(c);
	return 0;
}
