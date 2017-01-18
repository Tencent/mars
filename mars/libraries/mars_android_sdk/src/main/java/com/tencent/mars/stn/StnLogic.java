/**
 *  Created on: 2012-7-12
 *      Author: yerungui caoshaokun
 */
package com.tencent.mars.stn;

import com.tencent.mars.Mars;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

public class StnLogic {

    public static final String TAG = "mars.StnLogic";

    static {
        Mars.loadDefaultMarsLibrary();
    }


    public static class Task {
        public static final int ENORMAL = 0;
        public static final int EFAST = 1;

        //priority
        public static final int ETASK_PRIORITY_HIGHEST = 0;
        public static final int ETASK_PRIORITY_0 = 0;
        public static final int ETASK_PRIORITY_1 = 1;
        public static final int ETASK_PRIORITY_2 = 2;
        public static final int ETASK_PRIORITY_3 = 3;
        public static final int ETASK_PRIORITY_NORMAL = 3;
        public static final int ETASK_PRIORITY_4 = 4;
        public static final int ETASK_PRIORITY_5 = 5;
        public static final int ETASK_PRIORITY_LOWEST = 5;

        //channel selective
        public static final int EShort = 0x1;
        public static final int ELong = 0x2;
        public static final int EBoth = 0x3;
        private static AtomicInteger ai = new AtomicInteger(0);

        public Task() {
            this.taskID = ai.incrementAndGet();
        }

        public Task(final int channelselect, final int cmdid, final String cgi, final ArrayList<String> shortLinkHostList) {
            this.taskID = ai.incrementAndGet();
            this.channelSelect = channelselect;
            this.cmdID = cmdid;
            this.cgi = cgi;
            this.shortLinkHostList = shortLinkHostList;

            this.sendOnly = false;
            this.needAuthed = true;
            this.limitFlow = true;
            this.limitFrequency = true;

            this.channelStrategy = ENORMAL;
            this.networkStatusSensitive = false;
            this.priority = ETASK_PRIORITY_NORMAL;
            this.retryCount = -1;
            this.serverProcessCost = 0;
            this.totalTimeout = 0;
            this.userContext = null;
        }

        //require
        public int taskID;  //unique task identify
        public int channelSelect;   //short,long or both
        public int cmdID;
        public String cgi;
        public ArrayList<String> shortLinkHostList;    //host or ip

        //optional
        public boolean sendOnly;
        public boolean needAuthed;
        public boolean limitFlow;
        public boolean limitFrequency;

        public int channelStrategy;     //normal or fast
        public boolean networkStatusSensitive;
        public int priority;    //@see priority
        public int retryCount = -1;
        public int serverProcessCost;   //该TASK等待SVR处理的最长时间,也即预计的SVR处理耗时
        public int totalTimeout;    	//total timeout, in ms
        public Object userContext;      //user context
        public String reportArg;
    }

    public static final int INVALID_TASK_ID = -1;

    // STN callback errType
    public static final int ectOK = 0;
    public static final int ectFalse = 1;
    public static final int ectDial = 2;
    public static final int ectDns = 3;
    public static final int ectSocket = 4;
    public static final int ectHttp = 5;
    public static final int ectNetMsgXP = 6;
    public static final int ectEnDecode = 7;
    public static final int ectServer = 8;
    public static final int ectLocal = 9;

    //STN callback errCode
    public static final int FIRSTPKGTIMEOUT = -500;
    public static final int PKGPKGTIMEOUT = -501;
    public static final int READWRITETIMEOUT = -502;
    public static final int TASKTIMEOUT = -503;

    public static final int SOCKETNETWORKCHANGE = -10086;
    public static final int SOCKETMAKESOCKETPREPARED = -10087;
    public static final int SOCKETWRITENWITHNONBLOCK = -10088;
    public static final int SOCKETREADONCE = -10089;
    public static final int SOCKETSHUTDOWN = -10090;
    public static final int SOCKETRECVERR = -10091;
    public static final int SOCKETSENDERR = -10092;

    public static final int HTTPSPLITHTTPHEADANDBODY = -10194;
    public static final int HTTPPARSESTATUSLINE = -10195;

    public static final int NETMSGXPHANDLEBUFFERERR = -10504;

    public static final int DNSMAKESOCKETPREPARED = -10606;

    //reportConnectStatus
    //status
    public static final int NETWORK_UNKNOWN = -1;
    public static final int NETWORK_UNAVAILABLE = 0;
    public static final int GATEWAY_FAILED = 1;
    public static final int SERVER_FAILED = 2;
    public static final int CONNECTTING = 3;
    public static final int CONNECTED = 4;
    public static final int SERVER_DOWN = 5;

    //longlink identify check
    public static int ECHECK_NOW = 0;
    public static int ECHECK_NEXT = 1;
    public static int ECHECK_NEVER = 2;

    //buf2Resp fail handle type
    public static int RESP_FAIL_HANDLE_NORMAL = 0;
    public static int RESP_FAIL_HANDLE_DEFAULT = -1;
    public static int RESP_FAIL_HANDLE_SESSION_TIMEOUT = -13;
    public static int RESP_FAIL_HANDLE_TASK_END = -14;

    public static int TASK_END_SUCCESS = 0;



    private static ICallBack callBack = null;

    /**
     * 初始化网络层回调实例 App实现NetworkCallBack接口
     * @param _callBack native网络层调用java上层时的回调
     *
     */
    public static void setCallBack(ICallBack _callBack) {
        callBack = _callBack;
    }

    /**
     * 网络层调用java上层的接口类
     */

    /**
     * Created by caoshaokun on 16/2/1.
     *
     * APP使用信令通道必须实现该接口
     * 接口用于信令通道处理完后回调上层
     */
    public interface ICallBack {
        /**
         * SDK要求上层做认证操作(可能新发起一个AUTH CGI)
         * @return
         */
        boolean makesureAuthed();

        /**
         * SDK要求上层做域名解析.上层可以实现传统DNS解析,或者自己实现的域名/IP映射
         * @param host
         * @return
         */
        String[] onNewDns(final String host);

        /**
         * 收到SVR PUSH下来的消息
         * @param cmdid
         * @param data
         */
        void onPush(final int cmdid, final byte[] data);

        /**
         * SDK要求上层对TASK组包
         * @param taskID    任务标识
         * @param userContext
         * @param reqBuffer 组包的BUFFER
         * @param errCode   组包的错误码
         * @return
         */
        boolean req2Buf(final int taskID, Object userContext, ByteArrayOutputStream reqBuffer, int[] errCode, int channelSelect);

        /**
         * SDK要求上层对TASK解包
         * @param taskID        任务标识
         * @param userContext
         * @param respBuffer    要解包的BUFFER
         * @param errCode       解包的错误码
         * @return  int
         */
        int buf2Resp(final int taskID, Object userContext, final byte[] respBuffer, int[] errCode, int channelSelect);

        /**
         * 任务结束回调
         * @param taskID            任务标识
         * @param userContext
         * @param errType           错误类型
         * @param errCode           错误码
         * @return
         */
        int onTaskEnd(final int taskID, Object userContext, final int errType, final int errCode);

        /**
         * 流量统计
         * @param send
         * @param recv
         */
        void trafficData(final int send, final int recv);

        /**
         * 连接状态通知
         * @param status    综合状态，即长连+短连的状态
         * @param longlinkstatus    仅长连的状态
         */
        void reportConnectInfo(int status, int longlinkstatus);

        /**
         * SDK要求上层生成长链接数据校验包,在长链接连接上之后使用,用于验证SVR身份
         * @param identifyReqBuf    校验包数据内容
         * @param hashCodeBuffer    校验包的HASH
         * @param reqRespCmdID      数据校验的CMD ID
         * @return  ECHECK_NOW(需要校验), ECHECK_NEVER(不校验), ECHECK_NEXT(下一次再询问)
         */
        int getLongLinkIdentifyCheckBuffer(ByteArrayOutputStream identifyReqBuf, ByteArrayOutputStream hashCodeBuffer, int[] reqRespCmdID);

        /**
         * SDK要求上层解连接校验回包.
         * @param buffer            SVR回复的连接校验包
         * @param hashCodeBuffer    CLIENT请求的连接校验包的HASH值
         * @return
         */
        boolean onLongLinkIdentifyResp(final byte[] buffer, final byte[] hashCodeBuffer);

        /**
         * 请求做sync
         */
        void requestDoSync();
        String[] requestNetCheckShortLinkHosts();
        /**
         * 是否登录
         * @return true 登录 false 未登录
         */
        boolean isLogoned();

        void reportTaskProfile(String taskString);
    }

    /**
     * DEBUG IP 说明
     * setLonglinkSvrAddr,setShortlinkSvrAddr,setDebugIP 均可用于设置DEBUG IP
     * setLonglinkSvrAddr: 设置长链接的DEBUG IP;
     * setShortlinkSvrAddr: 设置短连接的DEBUG IP;
     * setDebugIP: 设置对应HOST(不区分长短链)的DEBUG IP;
     *
     * 优先级:
     * setDebugIP 为最高优先级
     * 同一个接口, 以最后设置的值为准
     */

    /**
     * @param host      长链接域名
     * @param ports     长链接端口列表
     * @param debugIP   长链接调试IP.如果有值,则忽略 host设置, 并使用该IP.
     */
    public static native void setLonglinkSvrAddr(final String host, final int[] ports, final String debugIP);
    public static        void setLonglinkSvrAddr(final String host, final int[] ports) {setLonglinkSvrAddr(host, ports, null);}

    /**
     *
     * @param port      短链接(HTTP)端口
     * @param debugIP   短链接调试IP.如果有值,则所有TASK走短链接时,使用该IP代替TASK中的HOST
     */
    public static native void setShortlinkSvrAddr(final int port, final String debugIP);

    public static        void setShortlinkSvrAddr(final int port) {setShortlinkSvrAddr(port, null);}

    /**
     * 设置DEBUG IP
     * @param host  要设置的域名
     * @param ip    该域名对应的IP
     */
    public static native void setDebugIP(final String host, final String ip);

    //async call
    public static native void startTask(final Task task);

    //sync call
    public static native void stopTask(final int taskID);

    //sync call
    public static native boolean hasTask(final int taskID);

    /**
     * 重做所有长短连任务. 注意这个接口会重连长链接.
     */
    public static native void redoTask();

    /**
     * 停止并清除所有未完成任务.
     */
    public static native void clearTask();

    /**
     * 停止并清除所有未完成任务并重新初始化
     */
    public static native void reset();

    /**
     * 设置备份IP,用于long/short svr均不可用的场景下
     * @param host  域名
     * @param ips   域名对应的IP列表
     */
    public static native void setBackupIPs(final String host, final String[] ips);

    /**
     * 检测长链接状态.如果没有连接上,则会尝试重连.
     */
    public static native void makesureLongLinkConnected();

    //signalling

    /**
     * 信令保活
     * @param period 信令保活间隔,默认5S
     * @param keepTime 信令保活时间,默认20S
     */
    public static native void setSignallingStrategy(long period, long keepTime);

    /**
     * 发送一个信令保活包(如果有必要)
     */
    public static native void keepSignalling();

    /**
     * 停止信令保活
     */
    public static native void stopSignalling();

    /**
     * 设置客户端版本 放入长连私有协议头部
     * @param clientVersion
     */
    public static native void setClientVersion(int clientVersion);

    /**
     * 获取底层已加载模块
     * @return
     */
    private static native ArrayList<String> getLoadLibraries();

    /**
     *  要求上层进行AUTH操作.
     *  如果一个TASK要求AUTH状态而当前没有AUTH态,组件就会回调此方法
     */
    private static boolean makesureAuthed() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return false;
            }
            return callBack.makesureAuthed();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * 长连host设置到网络层 网络层向上层请求host dns结果
     * 短连task中设置host  网络层向上层请求host dns结果
     * @param host  域名
     * @return 空：底层实现解析
     */
    private static String[] onNewDns(final String host) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return null;
            }
            return callBack.onNewDns(host);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 收到server push消息
     * @param cmdid     PUSH的CMDID,这个应该是APP跟SVR约定的值
     * @param data      PUSH下来的数据
     */
    private static void onPush(final int cmdid, final byte[] data) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }

            callBack.onPush(cmdid, data);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 网络层获取上层发送的数据内容
     * @param taskID
     * @param userContext
     * @param reqBuffer 数据
     * @param errCode 生成数据失败时的错误码
     * @return
     */

    private static boolean req2Buf(final int taskID, Object userContext, ByteArrayOutputStream reqBuffer, int[] errCode, int channelSelect) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return false;
            }
            return callBack.req2Buf(taskID, userContext, reqBuffer, errCode, channelSelect);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * 网络层将收到的信令回包交给上层解析
     * @param taskID
     * @param userContext
     * @param respBuffer 待解包的数据内容
     * @param errCode   解包错误码
     * @return
     */
    private static int buf2Resp(final int taskID, Object userContext, final byte[] respBuffer, int[] errCode, int channelSelect) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return RESP_FAIL_HANDLE_TASK_END;
            }
            return callBack.buf2Resp(taskID, userContext, respBuffer, errCode, channelSelect);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return RESP_FAIL_HANDLE_TASK_END;

    }

    /**
     * 信令回包网络层处理完毕回调上层
     * @param taskID
     * @param userContext
     * @param errType 参见 ErrCmdType
     * @param errCode
     * @return
     */
    private static int onTaskEnd(final int taskID, Object userContext, final int errType, final int errCode){
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return 0;
            }
            return callBack.onTaskEnd(taskID, userContext, errType, errCode);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return 0;
    }

    /**
     * 上报信令消耗的流量
     * @param send
     * @param recv
     */
	private static void trafficData(final int send, final int recv) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }
            callBack.trafficData(send, recv);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    /**
     * 网络层向上层反馈网络连接状态
     * @param status 综合状态，即长连+短连的状态
     *               kNetworkUnkown = -1, kNetworkUnavailable = 0, kGateWayFailed = 1, kServerFailed = 2, kConnecting = 3, kConnected = 4, kServerDown = 5
     * @param longlinkstatus  长连状态
     *                       kConnectIdle = 0, kConnecting = 1, kConnected, kDisConnected = 3, kConnectFailed = 4
     */
    private static void reportConnectStatus(int status, int longlinkstatus) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }
            callBack.reportConnectInfo(status, longlinkstatus);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 长连信令校验
     * @param reqBuf        CLIENT的校验数据
     * @param reqBufHash    CLIENT校验数据的HASH值
     * @param cmdID         校验包的CMDID
     * @return  ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
     */
    private static int getLongLinkIdentifyCheckBuffer(ByteArrayOutputStream reqBuf, ByteArrayOutputStream reqBufHash, int[] cmdID) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return ECHECK_NEVER;
            }
            return callBack.getLongLinkIdentifyCheckBuffer(reqBuf, reqBufHash, cmdID);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return ECHECK_NEVER;
    }

    /**
     * 长连信令校验回包
     * @param respBuf       SVR回复的校验应答数据
     * @param reqBufHash    对应的CLIENT校验数据HASH
     * @return true false
     */
    private static boolean onLongLinkIdentifyResp(final byte[] respBuf, final byte[] reqBufHash) {
        try {

            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return false;
            }
            return callBack.onLongLinkIdentifyResp(respBuf, reqBufHash);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    private static String[] requestNetCheckShortLinkHosts() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return null;
            }
            return callBack.requestNetCheckShortLinkHosts();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }


    public static void requestDoSync() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }
            callBack.requestDoSync();
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public static boolean isLogoned() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return false;
            }
            return callBack.isLogoned();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * Task运行完成时，STN将Task的运行时状态及统计数据返回给上层
     * @param taskString
     */
    private static void reportTaskProfile(String taskString) {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return;
            }
            callBack.reportTaskProfile(taskString);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}


