package com.tencent.mars.sample;

import android.app.Application;
import android.content.Context;
import android.os.Environment;

import com.tencent.mars.app.AppLogic;
import com.tencent.mars.sample.core.ActivityEvent;
import com.tencent.mars.sample.core.MainService;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.xlog.Log;
import com.tencent.mars.xlog.Xlog;

import java.io.File;
import java.util.Random;

/**
 *
 * Created by kirozhao on 16/1/18.
 */
public class SampleApplicaton extends Application {

    private static final String TAG = "Mars.SampleApplication";

    private MainService mainService;

    private static Context context;

    public static AppLogic.AccountInfo accountInfo = new AppLogic.AccountInfo(
            new Random(System.currentTimeMillis()/1000).nextInt(), "anonymous");

    public static volatile boolean hasSetUserName = false;

    @Override
    public void onCreate() {
        super.onCreate();
        context = this;

        System.loadLibrary("stlport_shared");
        System.loadLibrary("marsxlog");

        final String SDCARD = Environment.getExternalStorageDirectory().getAbsolutePath();
        final String logPath = SDCARD + "/marssample/log";

        //init xlog
        if (BuildConfig.DEBUG) {
            Xlog.appenderOpen(Xlog.LEVEL_DEBUG, Xlog.AppednerModeAsync, "", logPath, "MarsSample");
            Xlog.setConsoleLogOpen(true);

        } else {
            Xlog.appenderOpen(Xlog.LEVEL_INFO, Xlog.AppednerModeAsync, "", logPath, "MarsSample");
            Xlog.setConsoleLogOpen(false);
        }

        Log.setLogImp(new Xlog());

        // NOTE: MarsServiceProxy is for client/caller
        // Initialize MarsServiceProxy for local client, can be moved to other place
        MarsServiceProxy.init(this, getMainLooper(), null);
        mainService = new MainService(MarsServiceProxy.inst);
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

    public static Context getContext() {
        return context;
    }
}
