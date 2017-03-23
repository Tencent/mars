package com.tencent.mars.webserver;

import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.utils.LogUtils;

import org.apache.log4j.Logger;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.StreamingOutput;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import com.google.protobuf.ByteString;
import java.util.Random;

@Path("/mars/hello2")
public class HelloCgi {

    Logger logger = Logger.getLogger(EchoCgi.class.getName());

    @POST()
    @Consumes("*/*")
    @Produces("application/octet-stream")
    public Response hello(InputStream is) {
        try {
            final Main.HelloRequest request = Main.HelloRequest.parseFrom(is);

            logger.info(LogUtils.format("mars3 request from user=%s, text=%s", request.getUser(), request.getText()));
            if(request.hasDumpContent())
                logger.info(LogUtils.format("dump content length:%d", request.getDumpContent().size()));


            int retCode = 0;
            String errMsg = "hello mars";
            int size = 0;
            if(request.getText() == "64KB")
                size = 64*1024;
            if(request.getText() == "128KB")
                size = 128*1024;
            Main.HelloResponse response = null;
            if(size > 0) {
                byte[] dump = new byte[size];
                Random rand = new Random();
                rand.nextBytes(dump);
                response = Main.HelloResponse.newBuilder().setRetcode(retCode).setErrmsg(errMsg).setDumpContent(ByteString.copyFrom(dump)).build();
            } else {
                response = Main.HelloResponse.newBuilder().setRetcode(retCode).setErrmsg(errMsg).build();
            }
            final Main.HelloResponse resp = response;

            final StreamingOutput stream = new StreamingOutput() {
                public void write(OutputStream os) throws IOException {
                    resp.writeTo(os);
                }
            };
            return Response.ok(stream).header("Content-Length", response.getSerializedSize()).build();

        } catch (Exception e) {
            logger.info("error mars hello3");
            logger.info(LogUtils.format("%s", e));
        }

        return null;
    }
}
