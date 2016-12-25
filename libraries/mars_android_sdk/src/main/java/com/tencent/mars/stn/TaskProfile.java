package com.tencent.mars.stn;

import java.util.Arrays;

/**
 * 网络任务统计信息类
 * Created by caoshaokun on 2016/12/1.
 */

public class TaskProfile {

    public int taskId;
    public int cmdId;
    public String cgi;
    public long startTaskTime;
    public long endTaskTime;
    public int dyntimeStatus;
    public int errCode;
    public int errType;
    public int channelSelect;
    public ConnectProfile[] historyNetLinkers;

    public static class ConnectProfile {
        public long startTime;
        public long dnsTime;
        public long dnsEndTime;
        public long connTime;
        public int connErrCode;
        public int tryIPCount;
        public String ip;
        public int port;
        public String host;
        public int ipType;
        public long disconnTime;
        public long disconnErrType;
        public long disconnErrCode;

        @Override
        public String toString() {
            return "ConnectProfile{" +
                    "startTime=" + startTime +
                    ", dnsTime=" + dnsTime +
                    ", dnsEndTime=" + dnsEndTime +
                    ", connTime=" + connTime +
                    ", connErrCode=" + connErrCode +
                    ", tryIPCount=" + tryIPCount +
                    ", ip='" + ip + '\'' +
                    ", port=" + port +
                    ", host='" + host + '\'' +
                    ", ipType=" + ipType +
                    ", disconnTime=" + disconnTime +
                    ", disconnErrType=" + disconnErrType +
                    ", disconnErrCode=" + disconnErrCode +
                    '}';
        }
    }

    @Override
    public String toString() {
        return "TaskProfile{" +
                "taskId=" + taskId +
                ", cmdId=" + cmdId +
                ", cgi='" + cgi + '\'' +
                ", startTaskTime=" + startTaskTime +
                ", endTaskTime=" + endTaskTime +
                ", dyntimeStatus=" + dyntimeStatus +
                ", errCode=" + errCode +
                ", errType=" + errType +
                ", channelSelect=" + channelSelect +
                ", historyNetLinkers=" + Arrays.toString(historyNetLinkers) +
                '}';
    }
}


