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

import com.tencent.mars.sample.wrapper.remote.PushMessage;
import com.tencent.mars.sample.wrapper.remote.PushMessageHandler;

import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by caoshaokun on 16/12/20.
 */
public class MainService implements PushMessageHandler {

    public static String TAG = "Mars.Sample.MainService";

    private Thread recvThread;

    private LinkedBlockingQueue<PushMessage> pushMessages = new LinkedBlockingQueue<>();

    private BusinessHandler[] handlers = new BusinessHandler[]{
            new MessageHandler(),
            new StatisticHandler()
    };

    public MainService() {
        this.start();
    }

    public void start() {
        if (recvThread == null) {
            recvThread = new Thread(pushReceiver, "PUSH-RECEIVER");

            recvThread.start();
        }
    }

    private final Runnable pushReceiver = new Runnable() {
        @Override
        public void run() {
            while (true) {
                try {
                    PushMessage pushMessage = pushMessages.take();
                    if (pushMessage != null) {
                        for (BusinessHandler handler : handlers) {
                            if (handler.handleRecvMessage(pushMessage)) {
                                break;
                            }
                        }
                    }

                } catch (InterruptedException e) {
                    e.printStackTrace();
                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException e1) {
                        //
                    }
                }
            }
        }
    };

    @Override
    public void process(PushMessage message) {
        pushMessages.offer(message);
    }
}
