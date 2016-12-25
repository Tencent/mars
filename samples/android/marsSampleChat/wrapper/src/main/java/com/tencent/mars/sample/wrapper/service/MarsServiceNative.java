package com.tencent.mars.sample.wrapper.service;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;

import com.tencent.mars.BaseEvent;
import com.tencent.mars.Mars;
import com.tencent.mars.app.AppLogic;
import com.tencent.mars.sample.wrapper.remote.MarsRecvCallBack;
import com.tencent.mars.sample.wrapper.remote.MarsService;
import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sdt.SdtLogic;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.xlog.Log;

/**
 * Actually Mars Service running in main app
 * <p></p>
 * Created by kirozhao on 16/2/29.
 */
public class MarsServiceNative extends Service implements MarsService {

    private static final String TAG = "Mars.Sample.MarsServiceNative";

    private MarsServiceStub stub;

    @Override
    public void send(MarsTaskWrapper taskWrapper, Bundle taskProperties) throws RemoteException {
        stub.send(taskWrapper, taskProperties);
    }

    @Override
    public void cancel(MarsTaskWrapper taskWrapper) throws RemoteException {
        stub.cancel(taskWrapper);
    }

    @Override
    public void setRecvCallBack(MarsRecvCallBack callBack) {
        stub.setRecvCallBack(callBack);
    }

    @Override
    public void setAccountInfo(long uin, String userName) {
        stub.setAccountInfo(uin, userName);
    }

    @Override
    public IBinder asBinder() {
        return stub;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        final MarsServiceProfile profile = createMarsServiceProfile();
        stub = new MarsServiceStub(this, profile);

        // set callback
        AppLogic.setCallBack(stub);
        StnLogic.setCallBack(stub);
        SdtLogic.setCallBack(stub);

        // Initialize the Mars PlatformComm
        Mars.init(getApplicationContext(), new Handler(Looper.getMainLooper()));

        // Initialize the Mars
        StnLogic.setLonglinkSvrAddr(profile.longLinkHost(), profile.longLinkPorts());
        StnLogic.setShortlinkSvrAddr(profile.shortLinkPort());
        StnLogic.setClientVersion(profile.productID());
        Mars.onCreate(true);

        StnLogic.makesureLongLinkConnected();

        //
        Log.d(TAG, "mars service native created");
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "mars service native destroying");

        // Leave Mars
        Mars.onDestroy();
//        ContentDistributionNetwork.onDestroy();

        Log.d(TAG, "mars service native destroyed");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return stub;
    }

    public MarsServiceProfile createMarsServiceProfile() {
        return new DebugMarsServiceProfile();
    }
}
