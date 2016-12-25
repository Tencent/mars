package com.tencent.mars.sample.wrapper.remote;

/**
 * Created by shockcao on 16/12/20.
 */
public class PushMessage {

    public PushMessage(int cmdId, byte[] buffer) {
        this.cmdId = cmdId;
        this.buffer = buffer;
    }

    public int cmdId;

    public byte[] buffer;

}
