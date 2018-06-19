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

package com.tencent.mars.sample;

import android.app.ActivityManager;
import android.app.Application;
import android.content.Context;
import android.os.Environment;

import com.tencent.mars.app.AppLogic;
import com.tencent.mars.sample.core.ActivityEvent;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.service.DebugMarsServiceProfile;
import com.tencent.mars.sample.wrapper.service.MarsServiceNative;
import com.tencent.mars.sample.wrapper.service.MarsServiceProfile;
import com.tencent.mars.sample.wrapper.service.MarsServiceProfileFactory;
import com.tencent.mars.xlog.Log;
import com.tencent.mars.xlog.Xlog;

import java.util.Random;

/**
 * Created by zhaoyuan on 16/1/18.
 */
public class SampleApplicaton extends Application {

    private static final String TAG = "Mars.SampleApplication";

    private static Context context;

    public static AppLogic.AccountInfo accountInfo = new AppLogic.AccountInfo(
            new Random(System.currentTimeMillis() / 1000).nextInt(), "anonymous");

    public static volatile boolean hasSetUserName = false;

    private static class SampleMarsServiceProfile extends DebugMarsServiceProfile {

        @Override
        public String longLinkHost() {
            return "marsopen.cn";
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        context = this;

        System.loadLibrary("stlport_shared");
        System.loadLibrary("marsxlog");
        openXlog();

        MarsServiceNative.setProfileFactory(new MarsServiceProfileFactory() {
            @Override
            public MarsServiceProfile createMarsServiceProfile() {
                return new SampleMarsServiceProfile();
            }
        });

        // NOTE: MarsServiceProxy is for client/caller
        // Initialize MarsServiceProxy for local client, can be moved to other place
        MarsServiceProxy.init(this, getMainLooper(), null);
        MarsServiceProxy.inst.accountInfo = accountInfo;

        // Auto bind all activity event
        ActivityEvent.bind(getApplicationContext());

        Log.i(TAG, "application started");
    }

    @Override
    public void onTerminate() {
        Log.i(TAG, "application terminated");

        super.onTerminate();

        Log.appenderClose();

    }

    public static  void openXlog() {

        int pid = android.os.Process.myPid();
        String processName = null;
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo appProcess : am.getRunningAppProcesses()) {
            if (appProcess.pid == pid) {
                processName = appProcess.processName;
                break;
            }
        }

        if (processName == null) {
            return;
        }

        final String SDCARD = Environment.getExternalStorageDirectory().getAbsolutePath();
        final String logPath = SDCARD + "/marssample/log";

        String logFileName = processName.indexOf(":") == -1 ? "MarsSample" : ("MarsSample_" + processName.substring(processName.indexOf(":") + 1));

        if (BuildConfig.DEBUG) {
            Xlog.appenderOpen(Xlog.LEVEL_VERBOSE, Xlog.AppednerModeAsync, "", logPath, logFileName, "");
            Xlog.setConsoleLogOpen(true);
        } else {
            Xlog.appenderOpen(Xlog.LEVEL_INFO, Xlog.AppednerModeAsync, "", logPath, logFileName, "");
            Xlog.setConsoleLogOpen(false);
        }
        Log.setLogImp(new Xlog());
    }

    public static Context getContext() {
        return context;
    }
}
