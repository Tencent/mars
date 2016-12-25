/* adler32.h -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2004 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#ifndef COMM_ADLER32_H_
#define COMM_ADLER32_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

unsigned long adler32(unsigned long adler, const unsigned char* buf, unsigned int len);
unsigned long adler32_combine(unsigned long adler1, unsigned long adler2, unsigned long len2);

#ifdef __cplusplus
}
#endif

#endif  // COMM_ADLER32_H_
