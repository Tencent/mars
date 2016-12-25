// MarsRecvCallBack.aidl
package com.tencent.mars.sample.wrapper.remote;

// Declare any non-default types here with import statements

interface MarsRecvCallBack {

    void onRecv(int cmdId, inout byte[] buffer);

}
