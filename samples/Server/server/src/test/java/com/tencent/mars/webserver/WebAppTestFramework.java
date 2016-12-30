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

import org.glassfish.jersey.client.ClientConfig;

import java.io.ByteArrayInputStream;
import java.io.InputStream;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.Entity;
import javax.ws.rs.core.MediaType;

/**
 *
 * Created by zhaoyuan on 16/2/25.
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
