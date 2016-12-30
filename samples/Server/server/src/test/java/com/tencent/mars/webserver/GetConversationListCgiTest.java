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

import com.tencent.mars.misc.Log;
import com.tencent.mars.sample.proto.Main;

import junit.framework.Assert;

import org.junit.Test;

import java.util.List;

/**
 * Created by zhaoyuan on 16/2/25.
 */
public class GetConversationListCgiTest {

    @Test
    public void testGetConvList() throws Exception {
        final Main.ConversationListRequest request = Main.ConversationListRequest.newBuilder()
                .setAccessToken("")
                .setType(Main.ConversationListRequest.FilterType.DEFAULT_VALUE)
                .build();

        final Main.ConversationListResponse response = Main.ConversationListResponse.parseFrom(
                WebAppTestFramework.post("mars/getconvlist", request.toByteArray())
        );

        List<Main.Conversation> list = response.getListList();
        for (Main.Conversation c : list) {
            Log.d("test", "get conversation: name=[%s], topic=[%s], notice=[%s]", c.getName(), c.getTopic(), c.getNotice());
        }

        Assert.assertTrue(list.size() != 0);
    }
}
