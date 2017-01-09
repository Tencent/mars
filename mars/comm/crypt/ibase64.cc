
#include <stdlib.h>
#include <string.h>

#include "comm/crypt/ibase64.h"

namespace Comm {

static void init_conversion_tables(void);

/*  Global variables used in this source file only */
static unsigned char char_to_base64[128];
static char base64_to_char[64];
static int tables_initialised = 0;

int EncodeBase64(const unsigned char* sSrc, unsigned char* sTarget, const int nSize) {
    int target_size = 0;                /*  Length of target buffer          */
    int nb_block;                       /*  Total number of blocks           */
    unsigned char
    *p_source,                      /*  Pointer to source buffer         */
    *p_target,                      /*  Pointer to target buffer         */
    value;                          /*  Value of Base64 byte             */

    if (nSize == 0)
        return (0);

    if (!tables_initialised)
        init_conversion_tables();

    /*    Bit positions
                  | byte 1 | byte 2 | byte 3 |
    source block   87654321 87654321 87654321         -> 3 bytes of 8 bits

                  | byte 1 | byte 2 | byte 3 | byte 4 |
    Encoded block  876543   218765   432187   654321  -> 4 bytes of 6 bits
    */

    nb_block = (int)(nSize / 3);

    /*  Check if we have a partially-filled block                            */
    if (nb_block * 3 != (int) nSize)
        nb_block++;

    target_size = (int) nb_block * 4;
    sTarget [target_size] = '\0';

    p_source = (unsigned char*)sSrc;          /*  Point to start of buffers        */
    p_target = sTarget;

    while (nb_block--) {
        /*  Byte 1                                                           */
        value       = *p_source >> 2;
        *p_target++ = base64_to_char [value];

        /*  Byte 2                                                           */
        value = (*p_source++ & 0x03) << 4;

        if ((int)(p_source - sSrc) < nSize)
            value |= (*p_source & 0xF0) >> 4;

        *p_target++ = base64_to_char [value];

        /*  Byte 3 - pad the buffer with '=' if block not completed          */
        if ((int)(p_source - sSrc) < nSize) {
            value = (*p_source++ & 0x0F) << 2;

            if ((int)(p_source - sSrc) < nSize)
                value |= (*p_source & 0xC0) >> 6;

            *p_target++ = base64_to_char [value];
        } else
            *p_target++ = '=';

        /*  Byte 4 - pad the buffer with '=' if block not completed          */
        if ((int)(p_source - sSrc) < nSize) {
            value       = *p_source++ & 0x3F;
            *p_target++ = base64_to_char [value];
        } else
            *p_target++ = '=';
    }

    return (target_size);
}

int DecodeBase64(const unsigned char* sSrc, unsigned char* sTarget, const int nSize) {
    int
    target_size = 0;                /*  Length of target buffer          */
    int
    nb_block;                       /*  Total number of block            */
    unsigned char
    value,                          /*  Value of Base64 byte             */
    *p_source,                      /*  Pointer in source buffer         */
    *p_target;                      /*  Pointer in target buffer         */

    int        n;

    if (nSize == 0)
        return (0);

    if (!tables_initialised)
        init_conversion_tables();

    /*  Bit positions
                  | byte 1 | byte 2 | byte 3 | byte 4 |
    Encoded block  654321   654321   654321   654321  -> 4 bytes of 6 bits
                  | byte 1 | byte 2 | byte 3 |
    Decoded block  65432165 43216543 21654321         -> 3 bytes of 8 bits
    */

    nb_block    = nSize / 4;
    target_size = (int) nb_block * 3;
    sTarget [target_size] = '\0';

    p_source = (unsigned char*) sSrc;          /*  Point to start of buffers        */
    p_target = sTarget;

    while (nb_block--) {
        /*  Byte 1                                                           */
        *p_target    = char_to_base64 [(unsigned char) * p_source++] << 2;
        value        = char_to_base64 [(unsigned char) * p_source++];
        *p_target++ += ((value & 0x30) >> 4);

        /*  Byte 2                                                           */
        *p_target    = ((value & 0x0F) << 4);
        value        = char_to_base64 [(unsigned char) * p_source++];
        *p_target++ += ((value & 0x3C) >> 2);

        /*  Byte 3                                                           */
        *p_target    = (value & 0x03) << 6;
        value        = char_to_base64 [(unsigned char) * p_source++];
        *p_target++ += value;
    }

    // ����ĩβ�ж��ٸ�'='
    for (n = nSize - 1; n >= 0; --n) {
        if (sSrc[n] == '=')
            target_size--;
        else
            break;
    }

    return (target_size);
}

static void init_conversion_tables(void) {
    unsigned char
    value,                          /*  Value to store in table          */
    offset,
    index;                          /*  Index in table                   */

    /*  Reset the tables                                                     */
    memset(char_to_base64, 0, sizeof(char_to_base64));
    memset(base64_to_char, 0, sizeof(base64_to_char));

    value  = 'A';
    offset = 0;

    for (index = 0; index < 62; index++) {
        if (index == 26) {
            value  = 'a';
            offset = 26;
        } else if (index == 52) {
            value  = '0';
            offset = 52;
        }

        base64_to_char [index] = value + index - offset;
        char_to_base64 [value + index - offset] = index;
    }

    base64_to_char [62]  = '+';
    base64_to_char [63]  = '/';
    char_to_base64 ['+'] = 62;
    char_to_base64 ['/'] = 63;

    tables_initialised = 1;
}
}
