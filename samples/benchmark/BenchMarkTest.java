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

//package com.tencent.mars.sample;

import com.google.protobuf.nano.CodedOutputByteBufferNano;
import com.squareup.okhttp.MediaType;
import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.RequestBody;
import com.squareup.okhttp.Response;
import com.squareup.okhttp.ResponseBody;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.NanoMarsTaskWrapper;
import com.tencent.mars.xlog.Log;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.Random;

/**
 * Created by astrozhou on 17/1/13.
 */

public class BenchMarkTest {

    private static final String TAG = "Mars.Sample.BenchMark";
    private long start_time;
    private long task_time;
    private long task_cnt;
    private long task_suc;
    private long suc_time;

    private long okhttp_start_time;
    private long okhttp_task_time;
    private long okhttp_task_cnt;
    private long okhttp_task_suc;
    private long okhttp_suc_time;

    public BenchMarkTest() {
    }

    public enum BenchMarkScene {
        SceneNormalTest,
        SceneSensitivity,
    }

    private BenchMarkScene scene;

    private class HelloTaskWrapper extends NanoMarsTaskWrapper<Main.HelloRequest, Main.HelloResponse> {

        public HelloTaskWrapper() {
            super(new Main.HelloRequest(), new Main.HelloResponse());
            request.user = "mars";
            request.text = Integer.toString((int)task_cnt);
            //BenchMark: request 64KB, 128KB
            /*request.dumpContent = new byte[64*1024];
            Random rand = new Random();
            rand.nextBytes(request.dumpContent);*/

            setLongChannelSupport(false);
            setShortChannelSupport(true);
        }

        @Override
        public void onPreEncode(Main.HelloRequest req) {
        }

        @Override
        public void onPostDecode(Main.HelloResponse response) {
        }

        @Override
        public void onTaskEnd(final int errType, final int errCode) {
            if(scene == BenchMarkScene.SceneSensitivity) {
                Log.i(TAG, "mars cost:" + (System.currentTimeMillis() - task_time) + "errtype:" + errType + ", errcode:" + errCode + ", msg:" + response.errmsg);
                return;
            }

            long curr = System.currentTimeMillis();
            task_cnt++;

            if(errType == 0 && errCode == 0 && response.retcode == 0) {
                task_suc++;
                suc_time += curr - task_time;
                //Log.i(TAG, "mars suc cost:" + (curr - task_time) + ", count:" + task_cnt + ", suc count:" + task_suc + ", msg:" + response.errmsg + ", total:" + (curr - start_time));
            } else {
                Log.e(TAG, "mars fail cost:" + (curr - task_time) + ", count:" + task_cnt + ", total:" + (curr - start_time));
            }

            if((task_suc%50) == 0) {
                Log.e(TAG, "mars total:" + (curr - start_time) + ", avg:" + (suc_time)/(double)task_suc + ",rate:" + task_suc/(double)task_cnt + ", count:" + task_cnt + ", task suc:" + task_suc);
            }

            if(task_suc < 500) {
                task_time = System.currentTimeMillis();
                HelloTaskWrapper task = new HelloTaskWrapper();
                MarsServiceProxy.send(task.setHttpRequest("118.89.24.72", "/mars/hello2").setCmdID(8));
            } else {
                String msg = "final mars total:" + (curr - start_time) + ", avg:" + (suc_time)/(double)task_suc + ",rate:" + task_suc/(double)task_cnt + ", count:" + task_cnt + ", suc:" + task_suc;
                Log.e(TAG, msg);
            }
        }

    }

    public void StartMarsTest() {
        scene = BenchMarkScene.SceneNormalTest;
        Log.i(TAG, "start mars");
        start_time = task_time = task_cnt = task_suc = suc_time = 0;
        start_time = System.currentTimeMillis();

        HelloTaskWrapper task = new HelloTaskWrapper();
        task_time = System.currentTimeMillis();
        MarsServiceProxy.send(task.setHttpRequest("118.89.24.72", "/mars/hello2").setCmdID(8));
    }

    public void StartOkHttpTest() {
        scene = BenchMarkScene.SceneNormalTest;
        Log.i(TAG, "start okhttp");
        okhttp_start_time = okhttp_task_time = okhttp_task_cnt = okhttp_task_suc = okhttp_suc_time = 0;
        okhttp_start_time = System.currentTimeMillis();

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(true) {
                    doOkHttpRequest();
                    if((okhttp_task_suc%50) == 0) {
                        Log.e(TAG, "http total:" + (System.currentTimeMillis() - okhttp_start_time) + ", avg:" + (okhttp_suc_time)/(double)okhttp_task_suc + ",rate:" + (okhttp_task_suc/(double)okhttp_task_cnt) + ", count:" + okhttp_task_cnt + ", suc count:" + okhttp_task_suc);
                    }
                    if(okhttp_task_suc >= 500) {
                        String msg = "final http total:" + (System.currentTimeMillis() - okhttp_start_time) + ", avg:" + (okhttp_suc_time)/(double)okhttp_task_suc + ",rate:" + (okhttp_task_suc/(double)okhttp_task_cnt) + ", count:" + okhttp_task_cnt + ", suc:" + okhttp_task_suc;
                        Log.e(TAG, msg);
                        return;
                    }
                }
            }
        });
        thread.start();

    }

    /*
     * benchmark scene: sensitivity test
     * step 1: set network 100% Loss
     * step 2: after 5s, 10s, 15s....40s, set network available
     * step 3: compare the cgi result
     */
    public void StartSensitivityTest() {
        scene = BenchMarkScene.SceneSensitivity;
        Log.i(TAG, "start sensitivity test");

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                doOkHttpRequest();
            }
        });
        thread.start();

        task_time = System.currentTimeMillis();
        HelloTaskWrapper task = new HelloTaskWrapper();
        MarsServiceProxy.send(task.setHttpRequest("118.89.24.72", "/mars/hello2").setCmdID(8));
    }

    void doOkHttpRequest() {
        ++okhttp_task_cnt;
        try {
            OkHttpClient client = new OkHttpClient();

            Main.HelloRequest req = new Main.HelloRequest();
            req.user = "okhttp";
            req.text = Integer.toString((int)okhttp_task_cnt);
            //Benchmark 64KB/128KB
            /*req.dumpContent = new byte[64*1024];
            Random rand = new Random();
            rand.nextBytes(req.dumpContent);*/

            final byte[] flatArray = new byte[req.getSerializedSize()];
            final CodedOutputByteBufferNano output = CodedOutputByteBufferNano.newInstance(flatArray);
            req.writeTo(output);
            RequestBody reqBody = RequestBody.create(MediaType.parse("application/octet-stream"), flatArray);

            //normal request
            Request request = new Request.Builder()
                    .url("http://118.89.24.72:8080/mars/hello2")
                    .addHeader("Cache-Control", "no-cache")
                    .addHeader("Content-Type", "application/octet-stream")
                    .addHeader("Connection", "close")
                    .addHeader("Accept", "*/*")
                    .post(reqBody)
                    .build();

            okhttp_task_time = System.currentTimeMillis();
            // Execute the request and retrieve the response.
            Response response = client.newCall(request).execute();

            ResponseBody body = response.body();
            Main.HelloResponse helloResp = Main.HelloResponse.parseFrom(body.bytes());
            body.close();

            long curr = System.currentTimeMillis();
            okhttp_suc_time += curr - okhttp_task_time;
            ++okhttp_task_suc;
            //Log.i(TAG, "http type:" + type + ", suc cost:" + (curr - okhttp_task_time) + ", count:" + okhttp_task_cnt + ", suc count:" + okhttp_task_suc + ", ctn:" + helloResp.errmsg);
        } catch(Exception e) {
            Log.e(TAG, "http fail cost:" + (System.currentTimeMillis() - okhttp_task_time) + ", count:" + okhttp_task_cnt);
        }
    }

}
