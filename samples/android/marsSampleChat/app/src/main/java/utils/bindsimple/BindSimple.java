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

package utils.bindsimple;

import android.app.Activity;
import android.content.res.Resources;
import android.view.View;

import java.lang.reflect.Field;

/**
 * Simple runtime binding
 * <p/>
 * Created by zhaoyuan on 16/2/29.
 */
public class BindSimple {

    private static final String TAG = "BindSimple";

    public static void bind(Activity context) {

        final Resources res = context.getResources();
        final Field[] fields = context.getClass().getDeclaredFields();

        for (Field f : fields) {
            Class<?> type = f.getType();

            // View Subclass ?
            if (!View.class.isAssignableFrom(type)) {
                continue;
            }

            // Find views by id
            try {
                BindView anno = f.getAnnotation(BindView.class);
                if (anno == null) {
                    continue;
                }

                f.setAccessible(true);

                if (anno.value() != 0) {
                    f.set(context, context.findViewById(anno.value()));

                } else if (anno.id() != null) {
                    int id = res.getIdentifier(anno.id(), "id", context.getPackageName());
                    if (id != 0) {
                        f.set(context, context.findViewById(id));
                    }
                }

            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }
}
