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

package com.tencent.mars.sample.wrapper.remote;

import android.os.Bundle;

import com.tencent.mars.sample.utils.print.BundleFormat;
import com.tencent.mars.sample.wrapper.TaskProperty;



/**
 * MarsTaskWrapper using nano protocol buffer encoding
 * <p></p>
 * Created by zhaoyuan on 16/2/29.
 */
public abstract class AbstractTaskWrapper extends MarsTaskWrapper.Stub {

    private Bundle properties = new Bundle();

    public AbstractTaskWrapper() {

        // Reflects task properties
        final TaskProperty taskProperty = this.getClass().getAnnotation(TaskProperty.class);
        if (taskProperty != null) {
            setHttpRequest(taskProperty.host(), taskProperty.path());
            setShortChannelSupport(taskProperty.shortChannelSupport());
            setLongChannelSupport(taskProperty.longChannelSupport());
            setCmdID(taskProperty.cmdID());
        }
    }

    @Override
    public Bundle getProperties() {
        return properties;
    }

    @Override
    public abstract void onTaskEnd(int errType, int errCode);

    public AbstractTaskWrapper setHttpRequest(String host, String path) {
        properties.putString(MarsTaskProperty.OPTIONS_HOST, ("".equals(host) ? null : host));
        properties.putString(MarsTaskProperty.OPTIONS_CGI_PATH, path);

        return this;
    }

    public AbstractTaskWrapper setShortChannelSupport(boolean support) {
        properties.putBoolean(MarsTaskProperty.OPTIONS_CHANNEL_SHORT_SUPPORT, support);
        return this;
    }

    public AbstractTaskWrapper setLongChannelSupport(boolean support) {
        properties.putBoolean(MarsTaskProperty.OPTIONS_CHANNEL_LONG_SUPPORT, support);
        return this;
    }

    public AbstractTaskWrapper setCmdID(int cmdID) {
        properties.putInt(MarsTaskProperty.OPTIONS_CMD_ID, cmdID);
        return this;
    }

    @Override
    public String toString() {
        return "AbsMarsTask: " + BundleFormat.toString(properties);
    }
}
