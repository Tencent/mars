package com.tencent.mars.webserver;

import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.utils.LogUtils;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import java.io.InputStream;
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
                conversationList = new LinkedList<>();

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

            final StreamingOutput stream = os -> {
                os.write(response.toByteArray());
            };
            return Response.ok(stream).build();

        } catch (Exception e) {
            logger.info(LogUtils.format("request invalid", e));
        }

        return null;
    }
}