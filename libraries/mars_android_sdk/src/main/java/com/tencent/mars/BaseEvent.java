package com.tencent.mars;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

/**
 * 基础事件通知类
 * Created by zhoudingpin on 16/2/2.
 */
public class BaseEvent {

    public static native void onCreate();
    public static native void onDestroy();
    public static native void onNetworkChange();
    public static native void onForeground(final boolean forground);
    public static native void onSingalCrash(int sig);
    public static native void onExceptionCrash();

    /**
     * 网络切换监听，客户端通过注册该广播通知mars stn网络切换
     */
    public static class ConnectionReceiver extends BroadcastReceiver {

        public static NetworkInfo lastActiveNetworkInfo = null;
        public static WifiInfo lastWifiInfo = null;
        public static boolean lastConnected = true;

        public static String TAG = "mars.ConnectionReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {

            if (context == null || intent == null) {
                return;
            }

            ConnectivityManager mgr = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo netInfo = null;
            try {
                netInfo = mgr.getActiveNetworkInfo();
            } catch (Exception e) {
                Log.i(TAG, "getActiveNetworkInfo failed.");
            }

            checkConnInfo(context, netInfo);
        }

        public void checkConnInfo(final Context context, final NetworkInfo activeNetInfo) {

            if (activeNetInfo == null) {
                lastActiveNetworkInfo = null;
                lastWifiInfo = null;
                BaseEvent.onNetworkChange();
            }
            else if (activeNetInfo.getDetailedState() != NetworkInfo.DetailedState.CONNECTED) {

                if (lastConnected) {
                    lastActiveNetworkInfo = null;
                    lastWifiInfo = null;
                    BaseEvent.onNetworkChange();
                }

                lastConnected = false;
            }
            else {
                if (isNetworkChange(context, activeNetInfo)) {
                    BaseEvent.onNetworkChange();
                }
                lastConnected = true;
            }

        }

        public boolean isNetworkChange(final Context context, final NetworkInfo activeNetInfo) {

            boolean isWifi = (activeNetInfo.getType() == ConnectivityManager.TYPE_WIFI);
            if (isWifi) {
                WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
                WifiInfo wi = wifiManager.getConnectionInfo();
                if (wi != null && lastWifiInfo != null && lastWifiInfo.getBSSID().equals(wi.getBSSID())
                        && lastWifiInfo.getSSID().equals(wi.getSSID())
                        && lastWifiInfo.getNetworkId() == wi.getNetworkId()) {
                    Log.w(TAG, "Same Wifi, do not NetworkChanged");
                    return false;
                }
                lastWifiInfo = wi;
            } else if (lastActiveNetworkInfo != null
                    && lastActiveNetworkInfo.getExtraInfo() != null && activeNetInfo.getExtraInfo() != null
                    && lastActiveNetworkInfo.getExtraInfo().equals(activeNetInfo.getExtraInfo())
                    && lastActiveNetworkInfo.getSubtype() == activeNetInfo.getSubtype()
                    && lastActiveNetworkInfo.getType() == activeNetInfo.getType()) {
                return false;

            } else if (lastActiveNetworkInfo != null
                    && lastActiveNetworkInfo.getExtraInfo() == null && activeNetInfo.getExtraInfo() == null
                    && lastActiveNetworkInfo.getSubtype() == activeNetInfo.getSubtype()
                    && lastActiveNetworkInfo.getType() == activeNetInfo.getType()) {
                Log.w(TAG, "Same Network, do not NetworkChanged");
                return false;
            }

            if (!isWifi) {
            }

            lastActiveNetworkInfo = activeNetInfo;


            return true;
        }

    }

}
