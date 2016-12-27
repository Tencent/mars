package com.tencent.mars.sdt;

import java.util.Arrays;

/**
 * 信令探测结果信息类
 * Created by caoshaokun on 16/12/1.
 */
public class SignalDetectResult {

    public ResultDetail[] details;

    public static class ResultDetail {

        public int detectType;

        public int errorCode;

        public int networkType;

        public String detectIP;

        public long connTime;

        public int port;

        public int rtt;

        public String rttStr;

        public int httpStatusCode;

        public int pingCheckCount;

        public String pingLossRate;

        public String dnsDomain;

        public String localDns;

        public String dnsIP1;

        public String dnsIP2;

        @Override
        public String toString() {
            return "ResultDetail{" +
                    "detectType=" + detectType +
                    ", errorCode=" + errorCode +
                    ", networkType=" + networkType +
                    ", detectIP='" + detectIP + '\'' +
                    ", connTime=" + connTime +
                    ", port=" + port +
                    ", rtt=" + rtt +
                    ", rttStr='" + rttStr + '\'' +
                    ", httpStatusCode=" + httpStatusCode +
                    ", pingCheckCount=" + pingCheckCount +
                    ", pingLossRate='" + pingLossRate + '\'' +
                    ", dnsDomain='" + dnsDomain + '\'' +
                    ", localDns='" + localDns + '\'' +
                    ", dnsIP1='" + dnsIP1 + '\'' +
                    ", dnsIP2='" + dnsIP2 + '\'' +
                    '}';
        }
    }

    @Override
    public String toString() {
        return "SignalDetectResult{" +
                "details=" + Arrays.toString(details) +
                '}';
    }
}
