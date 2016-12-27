package com.tencent.mars.sdt;

import com.tencent.mars.Mars;

import java.util.ArrayList;

/**
 * 信令探测工具类
 * Created by caoshaokun on 16/3/21.
 */
public class SdtLogic {

    public static final String TAG = "mars.SdtLogic";

    public static interface NetCheckType {
        int kPingCheck = 0;
        int kDnsCheck = 1;
        int kNewDnsCheck = 2;
        int kTcpCheck = 3;
        int kHttpCheck = 4;
    };

    public static interface TcpCheckErrCode {
        int kTcpSucc      = 0;
        int kTcpNonErr    = 1;
        int kSelectErr    = -1;
        int kPipeIntr     = -2;
        int kSndRcvErr    = -3;
        int kAssertErr    = -4;
        int kTimeoutErr   = -5;
        int kSelectExpErr = -6;
        int kPipeExp      = -7;
        int kConnectErr   = -8;
        int kTcpRespErr	  = -9;
    };

    static {
        Mars.loadDefaultMarsLibrary();
    }

    /**
     * 信令探测回调接口，启动信令探测
     */
    public static interface ICallBack {

        void reportSignalDetectResults(String resultsJson);

    }

    private static ICallBack callBack = null;

    /**
     * 设置信令探测回调实例，探测结果将通过该实例通知上层
     * @param _callBack
     */
    public static void setCallBack(ICallBack _callBack) {
        callBack = _callBack;
    }

    /**
     * 设置一个Http连通状态探测的URI
     * @param requestURI
     */
    public static native void setHttpNetcheckCGI(String requestURI);

    /**
     * 获取底层已加载模块
     * @return
     */
    private static native ArrayList<String> getLoadLibraries();

    private static void reportSignalDetectResults(String resultsJson) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }
            callBack.reportSignalDetectResults(resultsJson);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

}
