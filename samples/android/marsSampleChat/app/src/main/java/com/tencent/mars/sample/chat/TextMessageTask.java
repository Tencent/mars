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

package com.tencent.mars.sample.chat;

import android.os.Handler;
import android.os.Looper;

import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.chat.proto.Chat;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.sample.wrapper.TaskProperty;
import com.tencent.mars.sample.wrapper.remote.NanoMarsTaskWrapper;

/**
 * Text messaging task
 * <p/>
 * Created by zhaoyuan on 16/2/29.
 */
@TaskProperty(
        host = "marsopen.cn",
        path = "/mars/sendmessage",
        cmdID = Main.CmdID.CMD_ID_SEND_MESSAGE_VALUE,
        longChannelSupport = true,
        shortChannelSupport = false
)
public class TextMessageTask extends NanoMarsTaskWrapper<Chat.SendMessageRequest.Builder, Chat.SendMessageResponse.Builder> {

    private Runnable callback = null;

    private Runnable onOK = null;
    private Runnable onError = null;

    private Handler uiHandler = new Handler(Looper.getMainLooper());

    public TextMessageTask(String topicName, String text) {
        super(Chat.SendMessageRequest.newBuilder(), Chat.SendMessageResponse.newBuilder());

        // TODO: Better not holding vm

        request.setAccessToken("test_token");
        request.setFrom(SampleApplicaton.accountInfo.userName);
        request.setTo("all");
        request.setText(text);
        request.setTopic(topicName);

    }

    @Override
    public void onPreEncode(Chat.SendMessageRequest.Builder request) {
        // TODO: Not thread-safe here
    }

    @Override
    public void onPostDecode(Chat.SendMessageResponse.Builder response) {
        if (response.getErrCode() == Chat.SendMessageResponse.Error.ERR_OK_VALUE) {
            callback = onOK;

        } else {
            callback = onError;
        }
    }

    @Override
    public void onTaskEnd(int errType, int errCode) {
        if (callback == null) {
            callback = onError;
        }

        uiHandler.post(callback);
    }

    public TextMessageTask onOK(Runnable onOK) {
        this.onOK = onOK;
        return this;
    }

    public TextMessageTask onError(Runnable onError) {
        this.onError = onError;
        return this;
    }
}
