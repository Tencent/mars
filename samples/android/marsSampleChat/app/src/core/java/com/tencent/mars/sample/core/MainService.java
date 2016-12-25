package com.tencent.mars.sample.core;

import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.PushMessage;
import com.tencent.mars.xlog.Log;

/**
 * Created by shockcao on 16/12/20.
 */
public class MainService {

    public static String TAG = "Mars.Sample.MainService";

    private Thread recvThread;

    private MarsServiceProxy inst;

    private BusinessHandler[] handlers = new BusinessHandler[] {
            new MessageHandler(),
            new StatisticHandler()
    };

    public MainService(MarsServiceProxy inst) {
        this.inst = inst;
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

                PushMessage pushMessage = inst.getPushMessage();
                if (pushMessage != null) {

                    for (BusinessHandler handler : handlers) {
                        if (handler.handleRecvMessage(pushMessage)) {
                            break;
                        }
                    }
                }
                else {
                    inst.queueWait();
                }
            }
        }
    };

}
