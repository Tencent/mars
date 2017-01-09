package com.tencent.mars;

import android.content.Context;
import android.os.Handler;

import com.tencent.mars.comm.PlatformComm;
import com.tencent.mars.xlog.Log;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * Created by caoshaokun on 16/2/1.
 */
public class Mars {

    public static void loadDefaultMarsLibrary(){
        try {
            System.loadLibrary("stlport_shared");
            System.loadLibrary("marsxlog");
            System.loadLibrary("marsstn");
        }
        catch (Throwable e) {
            Log.e("mars.Mars", "", e);
        }
    }

    private static volatile boolean hasInitialized  = false;

    /**
     * APP创建时初始化平台回调 必须在onCreate方法前调用
     * @param _context
     * @param _handler
     */
    public static void init(Context _context, Handler _handler) {
        PlatformComm.init(_context, _handler);
        hasInitialized = true;
    }

    /**
     * APP启动时首次调用onCreate前必须显示调用 init(Context _context, Handler _handler)方法 和
     * SignalTransmitNetwork设置长连接和短连接域名ip
     * @param isFirstStartup 是否首次进行mars create
     */
    public static void onCreate(boolean isFirstStartup) {
        if (isFirstStartup && hasInitialized) {
            BaseEvent.onCreate();
        }
        else if (!isFirstStartup) {
            BaseEvent.onCreate();
        }
        else {
            /**
             * 首次启动但未调用init 无法进行BaseEvent create
             */
            throw new IllegalStateException("function MarsCore.init must be executed before Mars.onCreate when application firststartup.");
        }
    }

    /**
     *  APP退出时 销毁组件
     */
    public static void onDestroy() {
        BaseEvent.onDestroy();
    }

}
