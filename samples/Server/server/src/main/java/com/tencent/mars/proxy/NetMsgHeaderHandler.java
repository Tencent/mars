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

package com.tencent.mars.proxy;

import com.tencent.mars.logicserver.ProxySession;
import com.tencent.mars.logicserver.TopicChats;
import com.tencent.mars.sample.chat.proto.Chat;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.utils.LogUtils;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.glassfish.jersey.client.ClientConfig;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.Entity;
import javax.ws.rs.core.MediaType;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufInputStream;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.ReferenceCountUtil;

/**
 * Created by zhaoyuan on 16/2/2.
 */
public class NetMsgHeaderHandler extends ChannelInboundHandlerAdapter {

    public static Logger logger = Logger.getLogger(NetMsgHeaderHandler.class.getName());

    private static Map<Integer, String> CMD_PATH_MAP = new HashMap<>();

    static {
        CMD_PATH_MAP.put(Main.CmdID.CMD_ID_HELLO_VALUE, "mars/hello");
        CMD_PATH_MAP.put(Main.CmdID.CMD_ID_SEND_MESSAGE_VALUE, "/mars/sendmessage");
        CMD_PATH_MAP.put(Main.CmdID.CMD_ID_HELLO2_VALUE, "/mars/hello2");
    }

    private ConcurrentHashMap<ChannelHandlerContext, Long> linkTimeout = new ConcurrentHashMap<>();
    private ContextTimeoutChecker checker;

    public NetMsgHeaderHandler() {
        super();

        checker = new ContextTimeoutChecker();
        Timer timer = new Timer();
        timer.schedule(checker, 15 * 60 * 1000, 15 * 60 * 1000);
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);

        logger.info("client connected! " + ctx.toString());
        linkTimeout.put(ctx, System.currentTimeMillis());
        TopicChats.getInstance().joinTopic(ctx);
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        try {
            // decode request
            final NetMsgHeader msgXp = new NetMsgHeader();
            final InputStream socketInput = new ByteBufInputStream((ByteBuf) msg);
            boolean ret = msgXp.decode(socketInput);
            IOUtils.closeQuietly(socketInput);

            if(!ret) return;

            linkTimeout.remove(ctx);
            linkTimeout.put(ctx, System.currentTimeMillis());
            logger.info(LogUtils.format("client req, cmdId=%d, seq=%d", msgXp.cmdId, msgXp.seq));

            final ProxySession proxySession = ProxySession.Manager.get(ctx);
            if (proxySession == null) {
            }

            String webCgi = CMD_PATH_MAP.get(msgXp.cmdId);
            switch (msgXp.cmdId) {
                case Main.CmdID.CMD_ID_HELLO_VALUE:
                    InputStream requestDataStream = new ByteArrayInputStream(msgXp.body);
                    InputStream inputStream = doHttpRequest(webCgi, requestDataStream);
                    if (inputStream != null) {
                        msgXp.body = IOUtils.toByteArray(inputStream);
                        IOUtils.closeQuietly(requestDataStream);
                        byte[] respBuf = msgXp.encode();
                        logger.info(LogUtils.format( "client resp, cmdId=%d, seq=%d, resp.len=%d", msgXp.cmdId, msgXp.seq, msgXp.body == null ? 0 : msgXp.body.length));
                        ctx.writeAndFlush(ctx.alloc().buffer().writeBytes(respBuf));
                    }
                    else {

                    }
                    break;
                case Main.CmdID.CMD_ID_HELLO2_VALUE:
                    requestDataStream = new ByteArrayInputStream(msgXp.body);

                    inputStream = doHttpRequest(webCgi, requestDataStream);
                    if (inputStream != null) {
                        msgXp.body = IOUtils.toByteArray(inputStream);
                        IOUtils.closeQuietly(requestDataStream);
                        byte[] respBuf = msgXp.encode();
                        logger.info(LogUtils.format("client resp, cmdId=%d, seq=%d, resp.len=%d", msgXp.cmdId, msgXp.seq, msgXp.body == null ? 0 : msgXp.body.length));
                        ctx.writeAndFlush(ctx.alloc().buffer().writeBytes(respBuf));
                    }
                    else {

                    }
                    break;
                case Main.CmdID.CMD_ID_SEND_MESSAGE_VALUE:
                    requestDataStream = new ByteArrayInputStream(msgXp.body);

                    inputStream = doHttpRequest(webCgi, requestDataStream);
                    if (inputStream != null) {
                        msgXp.body = IOUtils.toByteArray(inputStream);
                        Chat.SendMessageResponse response = Chat.SendMessageResponse.parseFrom(msgXp.body);
                        if (response != null && response.getErrCode() == Chat.SendMessageResponse.Error.ERR_OK_VALUE) {
                            TopicChats.getInstance().pushMessage(response.getTopic(), response.getText(), response.getFrom(), ctx);
                        }
                        IOUtils.closeQuietly(requestDataStream);
                        byte[] respBuf = msgXp.encode();
                        logger.info(LogUtils.format("client resp, cmdId=%d, seq=%d, resp.len=%d", msgXp.cmdId, msgXp.seq, msgXp.body == null ? 0 : msgXp.body.length));
                        ctx.writeAndFlush(ctx.alloc().buffer().writeBytes(respBuf));
                    }
                    else {

                    }
                    break;
                case NetMsgHeader.CMDID_NOOPING:
                    byte[] respBuf = msgXp.encode();
                    logger.info(LogUtils.format("client resp, cmdId=%d, seq=%d, resp.len=%d", msgXp.cmdId, msgXp.seq, msgXp.body == null ? 0 : msgXp.body.length));
                    ctx.writeAndFlush(ctx.alloc().buffer().writeBytes(respBuf));
                    break;
                default:
                    break;
            }
        }catch (Exception e) {
            e.printStackTrace();

        } finally {
            ReferenceCountUtil.release(msg);
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        // Close the connection when an exception is raised.
        cause.printStackTrace();
        ctx.close();
    }

    /**
     * redirect request to webserver
     * @param path
     * @param data
     * @return
     */
    private InputStream doHttpRequest(String path, InputStream data) {
        final Client client = ClientBuilder.newClient(new ClientConfig());
        final InputStream response = client.target("http://localhost:8080/")
                .path(path)
                .request(MediaType.APPLICATION_OCTET_STREAM)
                .post(Entity.entity(data, MediaType.APPLICATION_OCTET_STREAM), InputStream.class);

        return response;
    }

    /**
     *
     */
    public class ContextTimeoutChecker extends TimerTask {

        @Override
        public void run() {
            logger.info(LogUtils.format("check longlink alive per 15 minutes, " + this));
            for (ChannelHandlerContext context : linkTimeout.keySet()) {
                if (System.currentTimeMillis() - linkTimeout.get(context) > 15 * 60 * 1000) {
                    TopicChats.getInstance().leftTopic(context);
                    linkTimeout.remove(context);
                    logger.info(LogUtils.format("%s expired, deleted.", context.channel().toString()));
                }
            }
        }
    }
}

