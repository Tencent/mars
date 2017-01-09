package com.tencent.mars.app;

/**
 * APP的属性类
 * Created by caoshaokunon 16/3/7.
 */
public class AppLogic {

    public static final String TAG = "mars.AppLogic";

    /**
     * 帐号信息类
     */
    public static class AccountInfo{

        /**
         * 帐号
         */
        public long uin = 0;

        /**
         * 用户名
         */
        public String userName = "";

        public AccountInfo() {}

        public AccountInfo(long uin, String userName) {
            this.uin = uin;
            this.userName = userName;
        }

    }

    /**
     * 终端设备信息类
     */
    public static class DeviceInfo {

        /**
         * 设备名称
         */
        public String devicename = "";

        /**
         * 设备类型
         */
        public String devicetype = "";

        public DeviceInfo(String devicename, String devicetype) {
            this.devicename = devicename;
            this.devicetype = devicetype;
        }
    }

    /**
     * 关于APP信息的回调接口
     */
    public interface ICallBack {
        /**
         * STN 会将配置文件进行存储，如连网IPPort策略、心跳策略等，此类信息将会被存储在客户端上层指定的目录下
         * @return APP目录
         */
        String getAppFilePath();

        /**
         * STN 会根据客户端的登陆状态进行网络连接策略的动态调整，当用户非登陆态时，网络会将连接的频率降低
		 * 所以需要获取用户的帐号信息，判断用户是否已登录
         * @return 用户帐号信息
         */
        AccountInfo getAccountInfo();

        /**
         * 客户端版本号能够帮助 STN 清晰区分存储的网络策略配置文件。
         * @return 客户端版本号
         */
        int getClientVersion();

        /**
         * 客户端通过获取设备类型，加入到不同的上报统计回调中，供客户端进行数据分析
         * @return
         */
        DeviceInfo getDeviceType();
    }

    private static ICallBack callBack = null;

    /**
     * 设置mars回调接口实例，mars回调上层时会调用该实例的方法
     * @param _callback
     */
    public static void setCallBack(ICallBack _callback) {
        callBack = _callback;
    }

    /**
     * mars回调获取APP目录
     * @return
     */
    public static String getAppFilePath() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return null;
            }
            return callBack.getAppFilePath();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;

    }

    /**
     * mars回调获取用户帐号信息
     * @return
     */
    private static AccountInfo getAccountInfo() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return null;
            }
            return callBack.getAccountInfo();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * mars回调获取客户端版本号
     * @return
     */
    private static int getClientVersion() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return 0;
            }
            return callBack.getClientVersion();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return 0;
    }

    /**
     * mars回调获取终端设备信息
     * @return
     */
    private static DeviceInfo getDeviceType() {
        try {
            if (callBack == null) {
                new NullPointerException("callback is null").printStackTrace();
                return null;
            }
            return callBack.getDeviceType();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }
}
