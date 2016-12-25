package com.tencent.mars.sample.utils.print;

import android.os.Bundle;

import java.util.Set;

/**
 * Print bundle values
 * <p></p>
 * Created by kirozhao on 16/2/29.
 */
public class BundleFormat {

    public static String toString(Bundle bundle) {
        final StringBuilder sb = new StringBuilder("{");

        Set<String> keys = bundle.keySet();
        for (String k : keys) {
            Object obj = bundle.get(k);
            if (obj instanceof Bundle) {
                sb.append(toString((Bundle) obj));

            } else {
                sb.append(k).append("=").append(obj).append("; ");
            }
        }

        return sb.append("}").toString();
    }
}
