package com.tencent.mars.sample.utils.print;

/**
 *
 * Created by kirozhao on 2016/11/17.
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
