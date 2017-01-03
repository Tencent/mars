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

package com.tencent.mars.logicserver;

import com.tencent.mars.proxy.NetMsgHeader;
import com.tencent.mars.sample.chat.proto.Messagepush;
import com.tencent.mars.utils.BaseConstants;

import org.apache.log4j.Logger;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedDeque;

import io.netty.channel.ChannelHandlerContext;

/**
 * Created by caoshaokun on 16/12/19.
 */
public class TopicChats {

    public static Logger logger = Logger.getLogger(TopicChats.class.getName());

    private String[][] conDetails = new String[][]{
            new String[] {"Mars", "0", "STN Discuss"},
            new String[] {"Mars", "1", "Xlog Discuss"},
            new String[] {"Mars", "2", "SDT Discuss"}
    };

    public ConcurrentHashMap<String, ConcurrentLinkedDeque<ChannelHandlerContext>> topicJoiners = new ConcurrentHashMap<>();

    private static TopicChats topicChats = new TopicChats();

    public static TopicChats getInstance() {
        return topicChats;
    }

    private TopicChats() {

        for (int i = 0; i < conDetails.length; i++) {
            ConcurrentLinkedDeque<ChannelHandlerContext> ctxs = new ConcurrentLinkedDeque<>();
            topicJoiners.put(conDetails[i][1], ctxs);
        }

    }

    /**
     *
     * @param context
     */
    public void joinTopic(ChannelHandlerContext context) {
        for (String topicName : topicJoiners.keySet()) {
            if (!topicJoiners.get(topicName).contains(context)) {
                topicJoiners.get(topicName).offer(context);
            }
        }
    }

    /**
     *
     * @param context
     */
    public void leftTopic(ChannelHandlerContext context) {
        for (String topicName : topicJoiners.keySet()) {
            topicJoiners.get(topicName).remove(context);
        }
    }

    /**
     *
     * @param topicName
     * @param content
     * @param from
     * @param context
     */
    public void pushMessage(String topicName, String content, String from, ChannelHandlerContext context) {
        new Pusher(topicName, content, from, context).start();
    }


    /**
     *
     */
    private class Pusher extends Thread {

        private String topicName;
        private String content;
        private String from;
        private ChannelHandlerContext ctx;

        public Pusher(String topicName, String content, String from, ChannelHandlerContext ctx) {
            this.topicName = topicName;
            this.content = content;
            this.from = from;
            this.ctx = ctx;
        }

        @Override
        public void run() {
            try {
                ConcurrentLinkedDeque<ChannelHandlerContext> ctxs = topicJoiners.get(topicName);

                if (ctxs != null) {

                    Messagepush.MessagePush messagePush = Messagepush.MessagePush.newBuilder()
                            .setContent(content)
                            .setFrom(from)
                            .setTopic(topicName).build();
                    NetMsgHeader msgXp = new NetMsgHeader();
                    msgXp.cmdId = BaseConstants.MESSAGE_PUSH;
                    msgXp.body = messagePush.toByteArray();

                    for (ChannelHandlerContext context : ctxs) {
                        if (ctx != context) {
                            context.writeAndFlush(context.alloc().buffer().writeBytes(msgXp.encode()));
                        }
                    }

                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }

    }
}
