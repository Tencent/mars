package com.tencent.mars.sample.wrapper.remote;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;

import com.tencent.mars.Mars;
import com.tencent.mars.app.AppLogic;
import com.tencent.mars.xlog.Log;

import java.util.Objects;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Mars Service Proxy for component callers
 * <p></p>
 * Created by kirozhao on 16/2/26.
 */
public class MarsServiceProxy implements ServiceConnection {

    private static final String TAG = "Mars.Sample.MarsServiceProxy";

    // public static final String SERVICE_ACTION = "BIND_MARS_SERVICE";
    public static final String SERVICE_DEFUALT_CLASSNAME = "com.tencent.mars.sample.wrapper.service.MarsServiceNative";

    private MarsService service = null;

    private LinkedBlockingQueue<MarsTaskWrapper> queue = new LinkedBlockingQueue<>();

    private LinkedBlockingDeque<PushMessage> pushMessages = new LinkedBlockingDeque<>();
    private Object syncQueueLock = new Object();

    public static final ConcurrentHashMap<String, Integer> GLOBAL_CMD_ID_MAP = new ConcurrentHashMap<>();

    private static Context gContext;
    public static MarsServiceProxy inst;
    private static String gPackageName;
    private static String gClassName;

    private Worker worker;
    public AppLogic.AccountInfo accountInfo;

    private NanoMarsRecvCallBack recvCallBack = new NanoMarsRecvCallBack();

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

    public static void send(MarsTaskWrapper marsTaskWrapper) {
        inst.queue.offer(marsTaskWrapper);
    }

    public static void cancel(MarsTaskWrapper marsTaskWrapper) {
        inst.cancelSpecifiedTaskWrapper(marsTaskWrapper);
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
        Log.d(TAG, "remote mars service connected");

        try {
            service = MarsService.Stub.asInterface(iBinder);
            service.setRecvCallBack(recvCallBack);
            service.setAccountInfo(accountInfo.uin, accountInfo.userName);

        } catch (Exception e) {
            service = null;
        }
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
        try {
            service.setRecvCallBack(null);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        service = null;

        // TODO: need reconnect ?
        Log.d(TAG, "remote mars service disconnected");
    }

    public PushMessage getPushMessage() {
        return pushMessages.poll();
    }

    public void queueWait() {
        synchronized (syncQueueLock) {
            try {
                if (pushMessages.isEmpty()) {
                    syncQueueLock.wait();
                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void cancelSpecifiedTaskWrapper(MarsTaskWrapper marsTaskWrapper) {
        if (queue.remove(marsTaskWrapper)) {
            // Remove from queue, not exec yet, call MarsTaskWrapper::onTaskEnd
            try {
                marsTaskWrapper.onTaskEnd();

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
        }
        catch (Exception e) {

        }
    }

    private static class Worker extends Thread {

        @Override
        public void run() {

            while (true) {

                inst.continueProcessTaskWrappers();

            }

        }

    }

    private class NanoMarsRecvCallBack extends MarsRecvCallBack.Stub {

        @Override
        public void onRecv(int cmdId, byte[] buffer) throws RemoteException {

            pushMessages.offer(new PushMessage(cmdId, buffer));
            synchronized (syncQueueLock) {
                syncQueueLock.notify();
            }
        }
    }

}
