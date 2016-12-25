package com.tencent.mars.sample.chat;

import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;

import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.chat.proto.Chat;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.sample.wrapper.remote.NanoMarsTaskWrapper;
import com.tencent.mars.sample.wrapper.TaskProperty;

/**
 * Text messaging task
 * <p/>
 * Created by kirozhao on 16/2/29.
 */
@TaskProperty(
        host = "www.marsopen.cn",
        path = "/mars/sendmessage",
        cmdID = Main.CMD_ID_SEND_MESSAGE,
        longChannelSupport = true,
        shortChannelSupport = false
)
public class TextMessageTask extends NanoMarsTaskWrapper<Chat.SendMessageRequest, Chat.SendMessageResponse> {

    private Runnable callback = null;

    private Runnable onOK = null;
    private Runnable onError = null;

    private Handler uiHandler = new Handler(Looper.getMainLooper());

    public TextMessageTask(String topicName, String text) {
        super(new Chat.SendMessageRequest(), new Chat.SendMessageResponse());

        // TODO: Better not holding vm

        request.accessToken = "test_token";
        request.from = SampleApplicaton.accountInfo.userName;
        request.to = "all";
        request.text = text;
        request.topic = topicName;
    }

    @Override
    public void onPreEncode(Chat.SendMessageRequest request) {
        // TODO: Not thread-safe here
    }

    @Override
    public void onPostDecode(Chat.SendMessageResponse response) {
        if (response.errCode == Chat.SendMessageResponse.ERR_OK) {
            callback = onOK;

        } else {
            callback = onError;
        }
    }

    @Override
    public void onTaskEnd() {
        if (callback == null) {
            callback = onError;
        }

        uiHandler.post(callback);
        super.onTaskEnd();
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
