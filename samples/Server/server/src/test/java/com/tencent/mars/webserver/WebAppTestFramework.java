package com.tencent.mars.webserver;

import org.glassfish.jersey.client.ClientConfig;

import java.io.ByteArrayInputStream;
import java.io.InputStream;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.Entity;
import javax.ws.rs.core.MediaType;

/**
 *
 * Created by kirozhao on 16/2/25.
 */
public class WebAppTestFramework {

    public static InputStream post(String path, byte[] data) {
        return post("http://localhost:8080/", path, data);
    }

    public static InputStream post(String url, String path, byte[] data) {
        final Client client = ClientBuilder.newClient(new ClientConfig());

        return client.target(url)
                .path(path)
                .request(MediaType.APPLICATION_OCTET_STREAM)
                .post(Entity.entity(new ByteArrayInputStream(data), MediaType.APPLICATION_OCTET_STREAM), InputStream.class);
    }
}
