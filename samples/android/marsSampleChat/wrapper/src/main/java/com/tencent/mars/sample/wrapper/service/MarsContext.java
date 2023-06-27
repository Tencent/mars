package com.tencent.mars.sample.wrapper.service;

import com.tencent.mars.app.Context;

public class MarsContext {

    private static Object mObjectLock = new Object();
    private static Context sInstance;

    public static Context getInstance() {
        synchronized (mObjectLock) {
            if (sInstance == null) {
                sInstance = new Context("default");
            }
        }
        return sInstance;
    }

}
