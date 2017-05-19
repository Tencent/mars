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

import android.content.Intent;

import com.google.protobuf.nano.InvalidProtocolBufferNanoException;
import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.chat.proto.Messagepush;
import com.tencent.mars.sample.utils.Constants;
import com.tencent.mars.sample.wrapper.remote.PushMessage;
import com.tencent.mars.xlog.Log;

/**
 * Created by caoshaokun on 16/12/20.
 */
public class MessageHandler extends BusinessHandler {

    public static String TAG = MessageHandler.class.getSimpleName();

    @Override
    public boolean handleRecvMessage(PushMessage pushMessage) {

        switch (pushMessage.cmdId) {
            case Constants.PUSHCMD: {
                try {
                    Messagepush.MessagePush message = Messagepush.MessagePush.parseFrom(pushMessage.buffer);
                    Intent intent = new Intent();
                    intent.setAction(Constants.PUSHACTION);
                    intent.putExtra("msgfrom", message.from);
                    intent.putExtra("msgcontent", message.content);
                    intent.putExtra("msgtopic", message.topic);
                    SampleApplicaton.getContext().sendBroadcast(intent);
                } catch (InvalidProtocolBufferNanoException e) {
                    Log.e(TAG, "%s", e.toString());
                }
            }
            return true;
            default:
                break;
        }

        return false;
    }
}
