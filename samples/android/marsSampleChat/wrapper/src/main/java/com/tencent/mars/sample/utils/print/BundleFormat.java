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

import android.os.Bundle;

import java.util.Set;

/**
 * Print bundle values
 * <p></p>
 * Created by zhaoyuan on 16/2/29.
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
