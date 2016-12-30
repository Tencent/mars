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

package com.tencent.mars.sample.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;

import com.tencent.mars.xlog.Log;

import java.util.HashMap;
import java.util.Map;

/**
 * Event
 * <p/>
 * Created by zhaoyuan on 16/3/1.
 */
public class ActivityEvent {

    private static final String TAG = "Mars.Sample.ActivityEvent";

    public enum Connect {
        None,
        ChatActivity,
        ProfileActivity,
    }

    private static final Map<Connect, Class<? extends Activity>> CONNECT_CLASS_MAP;

    static {
        CONNECT_CLASS_MAP = new HashMap<>();
        CONNECT_CLASS_MAP.put(Connect.None, null); //
    }

    public static void bind(Context context) {
        ActivityInfo[] list = new ActivityInfo[0];

        try {
            list = context.getPackageManager()
                    .getPackageInfo(context.getPackageName(), PackageManager.GET_ACTIVITIES)
                    .activities;

        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        for (ActivityInfo ai : list) {
            try {
                Class<?> type = Class.forName(ai.name);
                if (!Activity.class.isAssignableFrom(type)) {
                    // This should not happen
                    continue;
                }

                ActivityEventConnection anno = type.getAnnotation(ActivityEventConnection.class);
                if (anno == null) {
                    return;
                }

                final Connect connect = anno.connect();
                if (connect != null && connect != Connect.None) {
                    CONNECT_CLASS_MAP.put(connect, (Class<? extends Activity>) type);
                    Log.d(TAG, "update connect: %s to class %s", connect, type);
                }

            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }
        }

    }

    public static void jumpTo(Context context, Connect connect, Intent intent) {
        Class<? extends Activity> activity = CONNECT_CLASS_MAP.get(connect);
        if (activity == null) {
            Log.e(TAG, "no connection set for %s", connect);
            return;
        }

        if (intent == null) {
            intent = new Intent(context, activity);

        } else {
            intent.setClass(context, activity);
        }

        context.startActivity(intent);
    }
}
