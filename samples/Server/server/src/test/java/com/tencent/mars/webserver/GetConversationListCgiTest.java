package com.tencent.mars.webserver;

import com.tencent.mars.misc.Log;
import com.tencent.mars.sample.proto.Main;

import junit.framework.Assert;

import org.junit.Test;

import java.util.List;

/**
 * Created by kirozhao on 16/2/25.
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
