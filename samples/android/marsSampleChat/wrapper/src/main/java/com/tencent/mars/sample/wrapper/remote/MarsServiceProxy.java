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

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;

import com.tencent.mars.app.AppLogic;
import com.tencent.mars.xlog.Log;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Mars Service Proxy for component callers
 * <p></p>
 * Created by zhaoyuan on 16/2/26.
 */
public class MarsServiceProxy implements ServiceConnection {

    private static final String TAG = "Mars.Sample.MarsServiceProxy";

    // public static final String SERVICE_ACTION = "BIND_MARS_SERVICE";
    public static final String SERVICE_DEFUALT_CLASSNAME = "com.tencent.mars.sample.wrapper.service.MarsServiceNative";

    private MarsService service = null;

    private LinkedBlockingQueue<MarsTaskWrapper> queue = new LinkedBlockingQueue<>();

    public static final ConcurrentHashMap<String, Integer> GLOBAL_CMD_ID_MAP = new ConcurrentHashMap<>();

    private static Context gContext;
    public static MarsServiceProxy inst;
    private static String gPackageName;
    private static String gClassName;

    private Worker worker;
    public AppLogic.AccountInfo accountInfo;

    private ConcurrentHashMap<Integer, PushMessageHandler> pushMessageHandlerHashMap = new ConcurrentHashMap<>();
    private MarsPushMessageFilter filter = new MarsPushMessageFilter.Stub() {

        @Override
        public boolean onRecv(int cmdId, byte[] buffer) throws RemoteException {
            PushMessageHandler handler = pushMessageHandlerHashMap.get(cmdId);
            if (handler != null) {
                Log.i(TAG, "processing push message, cmdid = %d", cmdId);
                PushMessage message = new PushMessage(cmdId, buffer);
                handler.process(message);
                return true;

            } else {
                Log.i(TAG, "no push message listener set for cmdid = %d, just ignored", cmdId);
            }

            return false;
        }
    };

    private MarsServiceProxy() {
        worker = new Worker();
        worker.start();
    }

    public static void init(Context context, Looper looper, String packageName) {
        if (inst != null) {
            // TODO: Already initialized
            return;
        }

        gContext = context.getApplicationContext();

        gPackageName = (packageName == null ? context.getPackageName() : packageName);
        gClassName = SERVICE_DEFUALT_CLASSNAME;

        inst = new MarsServiceProxy();
    }

    public static void setOnPushMessageListener(int cmdId, PushMessageHandler pushMessageHandler) {
        if (pushMessageHandler == null) {
            inst.pushMessageHandlerHashMap.remove(cmdId);
        } else {
            inst.pushMessageHandlerHashMap.put(cmdId, pushMessageHandler);
        }
    }

    public static void send(MarsTaskWrapper marsTaskWrapper) {
        inst.queue.offer(marsTaskWrapper);
    }

    public static void cancel(MarsTaskWrapper marsTaskWrapper) {
        inst.cancelSpecifiedTaskWrapper(marsTaskWrapper);
    }

    public void setForeground(boolean isForeground) {
        try {
            if (service == null) {
                Log.d(TAG, "try to bind remote mars service, packageName: %s, className: %s", gPackageName, gClassName);
                Intent i = new Intent().setClassName(gPackageName, gClassName);
                gContext.startService(i);
                if (!gContext.bindService(i, inst, Service.BIND_AUTO_CREATE)) {
                    Log.e(TAG, "remote mars service bind failed");
                }

                return;
            }
            service.setForeground(isForeground ? 1 : 0);
        }
        catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
        Log.d(TAG, "remote mars service connected");

        try {
            service = MarsService.Stub.asInterface(iBinder);
            service.registerPushMessageFilter(filter);
            service.setAccountInfo(accountInfo.uin, accountInfo.userName);

        } catch (Exception e) {
            service = null;
        }
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
        try {
            service.unregisterPushMessageFilter(filter);

        } catch (RemoteException e) {
            e.printStackTrace();
        }
        service = null;

        // TODO: need reconnect ?
        Log.d(TAG, "remote mars service disconnected");
    }

    private void cancelSpecifiedTaskWrapper(MarsTaskWrapper marsTaskWrapper) {
        if (queue.remove(marsTaskWrapper)) {
            // Remove from queue, not exec yet, call MarsTaskWrapper::onTaskEnd
            try {
                marsTaskWrapper.onTaskEnd(-1, -1);

            } catch (RemoteException e) {
                // Called in client, ignore RemoteException
                e.printStackTrace();
                Log.e(TAG, "cancel mars task wrapper in client, should not catch RemoteException");
            }

        } else {
            // Already sent to remote service, need to cancel it
            try {
                service.cancel(marsTaskWrapper);

            } catch (RemoteException e) {
                e.printStackTrace();
                Log.w(TAG, "cancel mars task wrapper in remote service failed, I'll make marsTaskWrapper.onTaskEnd");
            }
        }
    }

    private void continueProcessTaskWrappers() {
        try {
            if (service == null) {
                Log.d(TAG, "try to bind remote mars service, packageName: %s, className: %s", gPackageName, gClassName);
                Intent i = new Intent().setClassName(gPackageName, gClassName);
                gContext.startService(i);
                if (!gContext.bindService(i, inst, Service.BIND_AUTO_CREATE)) {
                    Log.e(TAG, "remote mars service bind failed");
                }

                // Waiting for service connected
                return;
            }

            MarsTaskWrapper taskWrapper = queue.take();
            if (taskWrapper == null) {
                // Stop, no more task
                return;
            }

            try {
                Log.d(TAG, "sending task = %s", taskWrapper);
                final String cgiPath = taskWrapper.getProperties().getString(MarsTaskProperty.OPTIONS_CGI_PATH);
                final Integer globalCmdID = GLOBAL_CMD_ID_MAP.get(cgiPath);
                if (globalCmdID != null) {
                    taskWrapper.getProperties().putInt(MarsTaskProperty.OPTIONS_CMD_ID, globalCmdID);
                    Log.i(TAG, "overwrite cmdID with global cmdID Map: %s -> %d", cgiPath, globalCmdID);
                }
                service.send(taskWrapper, taskWrapper.getProperties());

            } catch (Exception e) { // RemoteExceptionHandler
                e.printStackTrace();
            }
        } catch (Exception e) {

        }
    }

    private static class Worker extends Thread {

        @Override
        public void run() {

            while (true) {
                inst.continueProcessTaskWrappers();

                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                    //
                }
            }
        }
    }
}
