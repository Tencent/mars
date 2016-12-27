
/**
@file
@brief Base64º”√‹Ω‚√‹∫Ø ˝
  $Id$
@author $Author$
@date $Date$
@version $Revision$
@note Editor: tab=4
@note Platform: Linux
*/

#pragma once

namespace Comm {
    /**
     * Given a source string of length len, this returns the amount of
     * memory the destination string should have.
     *
     * remember, this is integer math
     * 3 bytes turn into 4 chars
     * ceiling[len / 3] * 4 + 1
     *
     * +1 is for any extra null.
     */
#define modp_b64_encode_len(A) ((A+2)/3 * 4 + 1)
    
    /**
     * Given a base64 string of length len,
     *   this returns the amount of memory required for output string
     *  It maybe be more than the actual number of bytes written.
     * NOTE: remember this is integer math
     * this allocates a bit more memory than traditional versions of b64
     * decode  4 chars turn into 3 bytes
     * floor[len * 3/4] + 2
     */
#define modp_b64_decode_len(A) (A / 4 * 3 + 2)
    
/**
    @brief Encode data buffer to base64 string.
    @param sSrc: Source data buffer.
    @param sTarget: To store decode string.
    @param nSize: The length of the data buffer.
    @return The length of target string.
*/
int EncodeBase64(const unsigned char* sSrc, unsigned char* sTarget, const int nSize);

/**
    @brief Decode base64 string to data.
    @param sSrc: Source base64 string.
    @param sTarget: To store data.
    @param nSize: The length of the data buffer.
    @return The length of target data.
*/
int DecodeBase64(const unsigned char* sSrc, unsigned char* sTarget, const int nSize);

}

