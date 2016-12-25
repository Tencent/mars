// IMarsService.aidl
package com.tencent.mars.sample.wrapper.remote;

// Declare any non-default types here with import statements

import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sample.wrapper.remote.MarsRecvCallBack;

interface MarsService {

    void send(MarsTaskWrapper taskWrapper, in Bundle taskProperties);

    void cancel(MarsTaskWrapper taskWrapper);

    void setRecvCallBack(MarsRecvCallBack recvCallBack);

    void setAccountInfo(in long uin, in String userName);
}
