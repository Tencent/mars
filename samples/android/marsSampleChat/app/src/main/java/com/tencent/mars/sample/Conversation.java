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

package com.tencent.mars.sample;

import android.databinding.BaseObservable;
import android.databinding.Bindable;

/**
 * Created by zhaoyuan on 16/2/18.
 */
public class Conversation extends BaseObservable {

    private String name;

    private String topic;
    private String notice;


    @Bindable
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }


    public Conversation(final String name, final String topic, final String notice) {
        this.name = name;
        this.topic = topic;
        this.notice = notice;
    }

    @Bindable
    public String getNotice() {
        return notice;
    }

    public void setNotice(String notice) {
        this.notice = notice;
        notifyPropertyChanged(com.tencent.mars.sample.BR.notice);
    }


    @Bindable
    public String getTopic() {
        return topic;
    }

    public void setTopic(String topic) {
        this.topic = topic;
        notifyPropertyChanged(com.tencent.mars.sample.BR.topic);
    }
}
