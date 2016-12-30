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

package com.tencent.mars.sample.wrapper.service;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;

import com.tencent.mars.Mars;
import com.tencent.mars.app.AppLogic;
import com.tencent.mars.sample.wrapper.remote.MarsPushMessageFilter;
import com.tencent.mars.sample.wrapper.remote.MarsService;
import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sdt.SdtLogic;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.xlog.Log;

/**
 * Actually Mars Service running in main app
 * <p></p>
 * Created by zhaoyuan on 16/2/29.
 */
public class MarsServiceNative extends Service implements MarsService {

    private static final String TAG = "Mars.Sample.MarsServiceNative";

    private MarsServiceStub stub;
    private static MarsServiceProfileFactory gFactory = new MarsServiceProfileFactory() {
        @Override
        public MarsServiceProfile createMarsServiceProfile() {
            return new DebugMarsServiceProfile();
        }
    };

    public static void setProfileFactory(MarsServiceProfileFactory factory) {
        gFactory = factory;
    }

    @Override
    public void send(MarsTaskWrapper taskWrapper, Bundle taskProperties) throws RemoteException {
        stub.send(taskWrapper, taskProperties);
    }

    @Override
    public void cancel(MarsTaskWrapper taskWrapper) throws RemoteException {
        stub.cancel(taskWrapper);
    }

    @Override
    public void registerPushMessageFilter(MarsPushMessageFilter filter) throws RemoteException {
        stub.registerPushMessageFilter(filter);
    }

    @Override
    public void unregisterPushMessageFilter(MarsPushMessageFilter filter) throws RemoteException {
        stub.unregisterPushMessageFilter(filter);
    }

    @Override
    public void setAccountInfo(long uin, String userName) {
        stub.setAccountInfo(uin, userName);
    }

    @Override
    public void setForeground(int isForeground) {stub.setForeground(isForeground);}

    @Override
    public IBinder asBinder() {
        return stub;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        final MarsServiceProfile profile = gFactory.createMarsServiceProfile();
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
}
