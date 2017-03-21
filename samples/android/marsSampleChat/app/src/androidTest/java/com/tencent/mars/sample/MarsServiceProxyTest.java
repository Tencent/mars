package com.tencent.mars.sample;

import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.SmallTest;

import com.google.gson.JsonObject;
import com.tencent.mars.sample.wrapper.TaskProperty;
import com.tencent.mars.sample.wrapper.remote.JsonMarsTaskWrapper;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.MarsTaskProperty;
import com.tencent.mars.xlog.Log;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.TimeUnit;

/**
 *
 * Created by zhaoyuan on 2017/3/20.
 */
@RunWith(AndroidJUnit4.class)
@SmallTest
public class MarsServiceProxyTest {

    private static final String TAG = "MarsServiceProxyTest";

    @TaskProperty(
            host = "marsopen.cn",
            path = "/mars/sendmessage"
    )
    class TestJsonMarsTaskWrapper extends JsonMarsTaskWrapper {

        TestJsonMarsTaskWrapper(JsonObject req, JsonObject resp) {
            super(req, resp);
        }

        @Override
        public void onPreEncode(JsonObject request) {
            // TODO:
        }

        @Override
        public void onPostDecode(JsonObject response) {
            // TODO:
        }

        @Override
        public void onTaskEnd(int errType, int errCode) {
            Log.d(TAG, "taskID = %d, error = (%d, %d)", getProperties().getInt(MarsTaskProperty.OPTIONS_TASK_ID), errType, errCode);
        }
    }

    @Test
    public void testSend() throws Exception {

        TestJsonMarsTaskWrapper[] testTasks = new TestJsonMarsTaskWrapper[10];
        for (int i = 0; i < 10; i++) {
            JsonObject req = new JsonObject();
            JsonObject resp = new JsonObject();
            testTasks[i] = new TestJsonMarsTaskWrapper(req, resp);
        }

        for (int i = 0; i < 10; i++) {
            MarsServiceProxy.send(testTasks[i]);
        }

        // TimeUnit.SECONDS.sleep(3);

        for (int i = 0; i < 10; i++) {
            MarsServiceProxy.cancel(testTasks[i]);
        }
    }

}
