/*
* Tencent is pleased to support the open source community by making Mars available.
* Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the MIT License (the "License"); you may not use this file except in 
* compliance with the License. You may obtain a copy of the License at
* http://opensource.org/licenses/MIT
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.tencent.mars.sample.utils.print;

/**
 *
 * Created by zhaoyuan on 2016/11/17.
 */

public class MemoryDump {

    public static final int MASK_4BIT = 0x0F;
    public static final int MASK_8BIT = 0xFF;
    public static final int MASK_16BIT = 0xFFFF;
    public static final int MASK_32BIT = 0xFFFFFFFF;

    public static String dumpHex(final byte[] privateKey) {
        if (privateKey == null) {
            return "(null)";
        }

        final char[] hexDigits = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

        final int j = privateKey.length;
        final int cHexWidth = 3;
        final char[] str = new char[j * cHexWidth];
        int k = 0;
        for (int i = 0; i < j; i++) {
            final byte byte0 = privateKey[i];
            str[k++] = ' ';
            str[k++] = hexDigits[byte0 >>> 4 & MASK_4BIT];
            str[k++] = hexDigits[byte0 & MASK_4BIT];
        }
        return new String(str);
    }
}
