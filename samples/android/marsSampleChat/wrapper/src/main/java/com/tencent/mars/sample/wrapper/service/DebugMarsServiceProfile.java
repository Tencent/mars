/*
* Tencent is pleased to support the open source community by making Mars available.
* Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the MIT License (the "License"); you may not use this file except in 
* compliance with the License. You may obtain a copy of the License at
* http://opensource.org/licenses/MIT
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.tencent.mars.sample.wrapper.service;

/**
 * Default profile for testing
 * <p>
 * Created by zhaoyuan on 2016/11/16.
 */

public class DebugMarsServiceProfile implements MarsServiceProfile {

    public static final short MAGIC = 0x0110;
    public static final short PRODUCT_ID = 200;
    public static final String LONG_LINK_HOST = "localhost";
    public static final int[] LONG_LINK_PORTS = new int[]{8081};
    public static final int SHORT_LINK_PORT = 8080;

    @Override
    public short magic() {
        return MAGIC;
    }

    @Override
    public short productID() {
        return PRODUCT_ID;
    }

    @Override
    public String longLinkHost() {
        return LONG_LINK_HOST;
    }

    @Override
    public int[] longLinkPorts() {
        return LONG_LINK_PORTS;
    }

    @Override
    public int shortLinkPort() {
        return SHORT_LINK_PORT;
    }
}
