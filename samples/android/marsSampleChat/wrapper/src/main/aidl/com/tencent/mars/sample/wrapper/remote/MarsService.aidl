// IMarsService.aidl
package com.tencent.mars.sample.wrapper.remote;

// Declare any non-default types here with import statements

import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sample.wrapper.remote.MarsPushMessageFilter;

interface MarsService {

    int send(MarsTaskWrapper taskWrapper, in Bundle taskProperties);

    void cancel(int taskID);

    void registerPushMessageFilter(MarsPushMessageFilter filter);

    void unregisterPushMessageFilter(MarsPushMessageFilter filter);

    void setAccountInfo(in long uin, in String userName);

    void setForeground(in int isForeground);
}
