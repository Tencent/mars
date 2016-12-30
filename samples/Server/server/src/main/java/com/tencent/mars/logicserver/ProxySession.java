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

import com.tencent.mars.datacenter.CacheData;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import io.netty.channel.ChannelHandlerContext;

/**
 * Created by zhaoyuan on 16/2/2.
 */
public class ProxySession {

    public static class Manager {

        private static Map<String, ChannelHandlerContext> channels = new HashMap<>();
        private static Map<String, PreparedStatement> cachedStatement = new HashMap<>();

        private static PreparedStatement statement(String sql) throws SQLException {
            PreparedStatement statement = cachedStatement.get(sql);
            if (statement == null) {
                statement = CacheData.prepareStatement(sql);
                cachedStatement.put(sql, statement);
            }
            return statement;
        }

        public static boolean connect() {
            try {

                final PreparedStatement statement = CacheData.prepareStatement(
                        "CREATE TABLE session (" +
                                "ctx TEXT PRIMARY KEY NOT NULL, " +
                                "uin INT, " +
                                "key TEXT )"
                );
                statement.execute();
                statement.close();

            } catch (SQLException e) {
                e.printStackTrace();
            }
            return false;
        }

        public static void disconnect() {
            try {
                final PreparedStatement statement = CacheData.prepareStatement(
                        "DROP TABLE session");

                statement.execute();

            } catch (SQLException e) {
                e.printStackTrace();
            }
        }

        public static ProxySession get(ChannelHandlerContext ctx) {
            final String name = ctx.name();

            try {
                final PreparedStatement s = statement("SELECT uin, key FROM session WHERE ctx is ?");
                s.setString(1, name);
                ResultSet rs = s.executeQuery();
                while (rs.next()) {
                    ProxySession session = new ProxySession();
                    session.uin = rs.getInt(1);
                    session.key = rs.getString(2);

                    return session;
                }

            } catch (SQLException e) {
                e.printStackTrace();
            }

            return null;
        }

        public static ChannelHandlerContext getChannelContextByUin(final int uin) {
            try {
                final PreparedStatement s = statement("SELECT ctx FROM session WHERE uin = ?");
                s.setInt(1, uin);
                ResultSet rs = s.executeQuery();
                if (rs.first()) {
                    return channels.get(rs.getString(1));
                }

            } catch (SQLException e) {
                e.printStackTrace();
            }

            return null;
        }

        public static ChannelHandlerContext getChannelContextByClientAddr(String clientAddr) {
            try{
                return channels.get(clientAddr);
            }
            catch (Exception e) {
                e.printStackTrace();
            }

            return null;
        }

        public static void updateChannelsByClientAddr(String clientAddr, ChannelHandlerContext context) {
            try {
                channels.put(clientAddr, context);
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }

        public static void updateByCTX(ProxySession session, ChannelHandlerContext ctx) {
            try {
                final PreparedStatement s = statement(
                        "REPLACE INTO session (" +
                                "ctx, " +
                                "uin, " +
                                "key" +
                                ") VALUES (?, ?, ?)");
                s.setString(1, ctx.name());
                s.setInt(2, session.uin);
                s.setString(3, session.key);
                s.executeUpdate();

            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }

    public int uin;
    public String key;
}
