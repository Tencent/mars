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

package com.tencent.mars.webserver;

import com.google.protobuf.InvalidProtocolBufferException;
import com.tencent.mars.sample.proto.Main;

import junit.framework.Assert;

import org.junit.Test;


/**
 *
 * Created by zhaoyuan on 16/2/2.
 */
public class EchoCgiTest {

    @Test
    public void testHello() throws Exception {

        final Main.HelloRequest request = Main.HelloRequest.newBuilder()
                .setUser("dkyang")
                .setText("hello")
                .build();

        final Main.HelloResponse response = Main.HelloResponse.parseFrom(
                WebAppTestFramework.post("mars/hello", request.toByteArray())
        );

        System.out.println(response.getErrmsg());

        Assert.assertTrue(response.getRetcode() == 0);
    }

    @Test
    public void testHelloFakeResponse() throws Exception {
        try {
            final Main.HelloResponse response = Main.HelloResponse.parseFrom(
                    WebAppTestFramework.post("mars/hello", new byte[100])
            );

        } catch (InvalidProtocolBufferException e) {
            e.printStackTrace();

            return;
        }

        Assert.assertTrue(false);
    }

}
