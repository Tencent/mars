// MarsTaskWrapper.aidl
package com.tencent.mars.sample.wrapper.remote;

// Declare any non-default types here with import statements

interface MarsTaskWrapper {

    Bundle getProperties(); // called locally

    byte[] req2buf();

    int buf2resp(in byte[] buf);

    void onTaskEnd(in int errType, in int errCode);
}
