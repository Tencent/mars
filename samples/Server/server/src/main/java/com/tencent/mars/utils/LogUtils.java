package com.tencent.mars.utils;

/**
 * Created by shockcao on 16/12/23.
 */
public class LogUtils {

    public static String format(final String fmt, Object... params) {
        String f;
        if (params != null) {
            f = String.format(fmt, params);
        } else {
            f = fmt;
        }

        return f;
    }

}
