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

/*
 *    Copyright 2016 Tencent WeChat
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.tencent.mars.sample.databinding;

import android.databinding.BindingAdapter;
import android.databinding.BindingConversion;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.EditText;

import com.tencent.mars.sample.R;

import utils.bindsimple.BindableString;

/**
 * Custom data binding
 * <p/>
 * Created by zhaoyuan on 16/3/7.
 */
public class CustomDataBinding {

    @BindingConversion
    public static String convertBindableToString(BindableString bindableString) {
        return bindableString.get();
    }

    @BindingAdapter("bindEditText")
    public static void bindEditText(EditText view, final BindableString bindableString) {
        if (view.getTag(R.id.tag_text_watcher_binding) == null) {
            view.setTag(R.id.tag_text_watcher_binding, true);

            view.addTextChangedListener(new TextWatcher() {
                @Override
                public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

                }

                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    bindableString.set(s.toString());
                }

                @Override
                public void afterTextChanged(Editable editable) {

                }
            });
        }

        String newValue = bindableString.get();
        if (!view.getText().toString().equals(newValue)) {
            view.setText(newValue);
        }
    }
}
