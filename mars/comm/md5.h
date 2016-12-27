

#ifndef COMM_MD5_H_
#define COMM_MD5_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MD5_CBLOCK    64
#define MD5_LBLOCK    (MD5_CBLOCK/4)
#define MD5_SIZE 16

typedef struct {
    unsigned int A, B, C, D;
    unsigned int Nl, Nh;
    unsigned int data[MD5_LBLOCK];
    unsigned int num;
} md5_t;

void    MD5_init(md5_t*);
void    MD5_process(md5_t*, const void*, unsigned int);
void    MD5_finish(md5_t*, void*);

void    MD5_buffer(const char* buffer, const unsigned int buf_len, void* signature);

void    MD5_sig_to_string(const void* signature, char str[2 * MD5_SIZE]);
void    MD5_sig_from_string(void* signature, const char str[2 * MD5_SIZE]);

#ifdef __cplusplus
}
#endif

#define MD5_CTX                  md5_t
#define Hash_MD5Init(A)       MD5_init(A)
#define Hash_MD5Update(A, B, C) MD5_process(A, B, C)
#define Hash_MD5Final(A, B)   MD5_finish(A, B)
#define Hash_MD5(A, B, C)          MD5_buffer(A, B, C)

#endif /*  COMM_MD5_H_ */
