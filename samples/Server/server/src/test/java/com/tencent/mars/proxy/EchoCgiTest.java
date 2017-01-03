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

import com.tencent.mars.misc.Log;
import com.tencent.mars.sample.proto.Main;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufInputStream;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.util.ReferenceCountUtil;
import org.junit.Test;

/**
 * Created by zhaoyuan on 16/2/2.
 */
public class EchoCgiTest {

    private static final String TAG = EchoCgiTest.class.getSimpleName();

    static class HelloClientHandler extends ChannelInboundHandlerAdapter {

        private final NetMsgHeader msgXp = new NetMsgHeader();

        // 接收server端的消息，并打印出来
        @Override
        public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
            try {
                msgXp.decode(new ByteBufInputStream((ByteBuf) msg));
                Log.i(TAG, "resp recevied! seq=%d", msgXp.seq);

                final Main.HelloResponse response = Main.HelloResponse.parseFrom(msgXp.body);
                Log.i(TAG, "resp decoded, resp.retcode=%d, resp.err=%s", response.getRetcode(), response.getErrmsg());

            } finally {
                ReferenceCountUtil.release(msg);
            }
        }

        // 连接成功后，向server发送消息
        @Override
        public void channelActive(ChannelHandlerContext ctx) throws Exception {
            Log.i(TAG, "connected to server!");

            final Main.HelloRequest request = Main.HelloRequest.newBuilder()
                    .setUser("zhaoyuan")
                    .setText("hello proxy")
                    .build();

            msgXp.cmdId = Main.CmdID.CMD_ID_HELLO_VALUE;
            msgXp.body = request.toByteArray();

            //
            byte[] toSendBuf = msgXp.encode();
            ByteBuf encoded = ctx.alloc().buffer(toSendBuf.length);
            encoded.writeBytes(toSendBuf);
            ctx.writeAndFlush(encoded);

            Log.i(TAG, "write and flush!");
        }
    }


    static class HelloClient {
        public void connect(String host, int port) {
            EventLoopGroup workerGroup = new NioEventLoopGroup();

            try {
                Bootstrap b = new Bootstrap();
                b.group(workerGroup);
                b.channel(NioSocketChannel.class);
                b.option(ChannelOption.SO_KEEPALIVE, true);
                b.handler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    public void initChannel(SocketChannel ch) throws Exception {
                        ch.pipeline().addLast(new HelloClientHandler());
                    }
                });

                // Start the client.
                ChannelFuture f = b.connect(host, port).sync();

                // Wait until the connection is closed.
                f.channel().closeFuture().sync();

            } catch (InterruptedException e) {
                e.printStackTrace();

            } finally {
                workerGroup.shutdownGracefully();
            }
        }

    }


    @Test
    public void testHelloProxy() {
        HelloClient client = new HelloClient();
        client.connect("127.0.0.1", 8081);
    }


}
