package com.tencent.mars.sample.wrapper.service;

/**
 * profile interface
 * <p>
 * Created by kirozhao on 2016/11/16.
 */

public interface MarsServiceProfile {

    short magic();
    
    short productID();

    String longLinkHost();

    int[] longLinkPorts();

    int shortLinkPort();
}
