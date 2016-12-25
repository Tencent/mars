package com.tencent.mars.sample.core;

import com.tencent.mars.sample.wrapper.remote.PushMessage;

/**
 * Created by shockcao on 16/12/20.
 */
public abstract class BusinessHandler {

    public abstract boolean handleRecvMessage(PushMessage pushMessage);

}
