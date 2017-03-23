package com.tencent.mars.sample;

import android.support.test.filters.LargeTest;
import android.support.test.runner.AndroidJUnit4;

import com.google.gson.JsonObject;
import com.tencent.mars.sample.wrapper.TaskProperty;
import com.tencent.mars.sample.wrapper.remote.JsonMarsTaskWrapper;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.MarsTaskProperty;
import com.tencent.mars.xlog.Log;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.TimeUnit;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.not;
import static org.hamcrest.MatcherAssert.assertThat;

/**
 * Created by zhaoyuan on 2017/3/20.
 */
@RunWith(AndroidJUnit4.class)
@LargeTest
public class MarsServiceProxyTest {

    private static final String TAG = "MarsServiceProxyTest";

    @TaskProperty(
        host = "marsopen.cn",
        path = "/mars/sendmessage"
    )
    class TestJsonMarsTaskWrapper extends JsonMarsTaskWrapper {

        String text;
        int errType = -1;
        int errCode = -1;

        TestJsonMarsTaskWrapper(String text) {
            super(new JsonObject(), new JsonObject());

            this.text = text;
        }

        @Override
        public void onPreEncode(JsonObject request) {
            request.addProperty("text", text);
            // TODO: sleep for a while for cancelling test
            try {
                TimeUnit.SECONDS.sleep(1);

            } catch (InterruptedException e) {
                //
            }
        }

        @Override
        public void onPostDecode(JsonObject response) {
            // TODO:
        }

        @Override
        public void onTaskEnd(int errType, int errCode) {
            Log.d(TAG, "%s taskID = %d, error = (%d, %d)", request.toString(), getProperties().getInt(MarsTaskProperty.OPTIONS_TASK_ID), errType, errCode);

            this.errType = errType;
            this.errCode = errCode;
        }
    }

    @Before
    public void setUp() {
    }

    @Test
    public void testCancelBeforeSent() throws Exception {
        TestJsonMarsTaskWrapper task = new TestJsonMarsTaskWrapper("cancel-before-send");

        // Send task
        MarsServiceProxy.send(task);

        // Then, cancel immediately
        MarsServiceProxy.cancel(task);

        assertThat(task.errType, is(-1));
        assertThat(task.errCode, is(-1));
    }

    @Test
    public void testCancelAfterSent() throws Exception {
        TestJsonMarsTaskWrapper task = new TestJsonMarsTaskWrapper("cancel-after-send");

        // Send task
        MarsServiceProxy.send(task);

        // Wait. I guess 15 seconds should be enough for local network
        TimeUnit.SECONDS.sleep(15);

        // Then, cancel
        MarsServiceProxy.cancel(task);

        assertThat(task.errType, not(-1));
        assertThat(task.errCode, not(-1));
    }

}
