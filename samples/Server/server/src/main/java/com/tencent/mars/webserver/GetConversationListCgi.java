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

import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.utils.LogUtils;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.StreamingOutput;


@Path("/mars/getconvlist")
public class GetConversationListCgi {

    static {
        Properties pro = new Properties();
        pro.put("log4j.rootLogger", "DEBUG,stdout,R");

        pro.put("log4j.appender.stdout", "org.apache.log4j.ConsoleAppender");
        pro.put("log4j.appender.stdout.layout", "org.apache.log4j.PatternLayout");
        pro.put("log4j.appender.stdout.layout.ConversionPattern", "%5p [%t] (%F:%L) - %m%n");

        pro.put("log4j.appender.R", "org.apache.log4j.DailyRollingFileAppender");
        pro.put("log4j.appender.R.Threshold", "INFO");
        pro.put("log4j.appender.R.File", "${user.home}/logs/mars/info_webserver.log");
        pro.put("log4j.appender.R.DatePattern", ".yyyy-MM-dd");
        pro.put("log4j.appender.R.layout", "org.apache.log4j.PatternLayout");
        pro.put("log4j.appender.R.layout.ConversionPattern", "[%d{HH:mm:ss:SSS}] [%p] - %l - %m%n");

        PropertyConfigurator.configure(pro);
    }

    Logger logger = Logger.getLogger(GetConversationListCgi.class.getName());

    private String[][] conDetails = new String[][]{
            new String[] {"Mars", "0", "STN Discuss"},
            new String[] {"Mars", "1", "Xlog Discuss"},
            new String[] {"Mars", "2", "SDT Discuss"}
    };

    @POST()
    @Consumes("application/octet-stream")
    @Produces("application/octet-stream")
    public Response getconvlist(InputStream is) {
        try {
            final Main.ConversationListRequest request = Main.ConversationListRequest.parseFrom(is);

            logger.info(LogUtils.format("request from access_token=%s, type=%d", request.getAccessToken(), request.getType()));

            List<Main.Conversation> conversationList = null;
            if (request.getType() == Main.ConversationListRequest.FilterType.DEFAULT_VALUE) {
                conversationList = new LinkedList<Main.Conversation>();

                for (String[] conv : conDetails) {
                    conversationList.add(Main.Conversation.newBuilder()
                            .setName(conv[0])
                            .setTopic(conv[1])
                            .setNotice(conv[2])
                            .build());
                }
            }

            final Main.ConversationListResponse response = Main.ConversationListResponse.newBuilder()
                    .addAllList(conversationList).build();

            final StreamingOutput stream = new StreamingOutput() {
                public void write(OutputStream os) throws IOException {
                    response.writeTo(os);
                }
            };
            return Response.ok(stream).build();

        } catch (Exception e) {
            logger.info(LogUtils.format("request invalid", e));
        }

        return null;
    }
}