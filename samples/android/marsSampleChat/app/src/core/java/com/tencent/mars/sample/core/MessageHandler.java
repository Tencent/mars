package com.tencent.mars.sample.core;

import android.content.Intent;

import com.google.protobuf.nano.InvalidProtocolBufferNanoException;
import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.chat.proto.Chat;
import com.tencent.mars.sample.chat.proto.Messagepush;
import com.tencent.mars.sample.utils.Constants;
import com.tencent.mars.sample.wrapper.remote.PushMessage;
import com.tencent.mars.xlog.Log;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by shockcao on 16/12/20.
 */
public class MessageHandler extends BusinessHandler{

    public static String TAG = MessageHandler.class.getSimpleName();

    @Override
    public boolean handleRecvMessage(PushMessage pushMessage) {

        switch (pushMessage.cmdId) {
            case Constants.PUSHCMD:
            {
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
