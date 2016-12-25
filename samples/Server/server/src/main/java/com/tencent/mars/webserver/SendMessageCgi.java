package com.tencent.mars.webserver;

import com.tencent.mars.sample.chat.proto.Chat;
import com.tencent.mars.utils.LogUtils;

import org.apache.log4j.Logger;

import java.io.InputStream;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.StreamingOutput;


@Path("/mars/sendmessage")
public class SendMessageCgi {

    Logger logger = Logger.getLogger(SendMessageCgi.class.getName());

    @POST()
    @Consumes("application/octet-stream")
    @Produces("application/octet-stream")
    public Response sendmessage(InputStream is) {
        try {
            final Chat.SendMessageRequest request = Chat.SendMessageRequest.parseFrom(is);

            logger.info(LogUtils.format("request from user=%s, text=%s", request.getFrom(), request.getText()));

            int retCode = Chat.SendMessageResponse.Error.ERR_OK_VALUE;
            String errMsg = "congratulations, " + request.getFrom();
            final Chat.SendMessageResponse response = Chat.SendMessageResponse.newBuilder()
                    .setErrCode(retCode)
                    .setErrMsg(errMsg)
                    .setFrom(request.getFrom())
                    .setText(request.getText())
                    .setTopic(request.getTopic())
                    .build();

            final StreamingOutput stream = os -> {
                os.write(response.toByteArray());
            };
            return Response.ok(stream).build();

        } catch (Exception e) {
            logger.info(LogUtils.format("%s", e));
        }

        return null;
    }
}