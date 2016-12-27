package com.tencent.mars.comm;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;

import com.tencent.mars.xlog.Log;

import java.util.List;

public class NetStatusUtil {

    private static final String TAG = "MicroMsg.NetStatusUtil";
    public static final int NON_NETWORK = -1;
    public static final int WIFI = 0;
    public static final int UNINET = 1;
    public static final int UNIWAP = 2;
    public static final int WAP_3G = 3;
    public static final int NET_3G = 4;
    public static final int CMWAP = 5;
    public static final int CMNET = 6;
    public static final int CTWAP = 7;
    public static final int CTNET = 8;
    public static final int MOBILE = 9;
    public static final int LTE = 10;

    /**
     * No specific network policy, use system default.
     */
    public static final int POLICY_NONE = 0x0;
    /**
     * Reject network usage on metered networks when application in background.
     */
    public static final int POLICY_REJECT_METERED_BACKGROUND = 0x1;

    public static final int TBACKGROUND_NOT_LIMITED = 0x0;
    public static final int TBACKGROUND_PROCESS_LIMITED = 0x1;
    public static final int TBACKGROUND_DATA_LIMITED = 0x2;
    public static final int TBACKGROUND_WIFI_LIMITED = 0x3;

    public static final int NO_SIM_OPERATOR = 0;

    public static void dumpNetStatus(Context context) {

        try {
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
            Log.i(TAG, activeNetInfo.toString());
        } catch (Exception e) {
            Log.e(TAG, "", e);
        }
    }

    public static boolean isConnected(Context context) {

        ConnectivityManager conMan = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetInfo = conMan.getActiveNetworkInfo();
        boolean connect = false;
        try {
            connect = activeNetInfo.isConnected();
        } catch (Exception e) {
        }
        return connect;
    }

    public static String getNetTypeString(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivityManager == null) {
            return "NON_NETWORK";
        }
        NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
        if (activeNetInfo == null) {
            return "NON_NETWORK";
        }

        if (activeNetInfo.getType() == ConnectivityManager.TYPE_WIFI) {
            return "WIFI";

        } else {
            if (activeNetInfo.getExtraInfo() != null) {
                return activeNetInfo.getExtraInfo();
            }
            return "MOBILE";
        }
    }


    public static int getNetWorkType(Context context) {

        try {

            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            if (netInfo != null) {
                return netInfo.getType();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return NON_NETWORK;
    }

    public static int getNetType(Context context) {

        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivityManager == null) {
            return NON_NETWORK;
        }
        NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
        if (activeNetInfo == null) {
            return NON_NETWORK;
        }

        if (activeNetInfo.getType() == ConnectivityManager.TYPE_WIFI) {
            return WIFI;

        } else {
            if (activeNetInfo.getExtraInfo() != null) {
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("uninet")) {
                    return UNINET;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("uniwap")) {
                    return UNIWAP;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("3gwap")) {
                    return WAP_3G;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("3gnet")) {
                    return NET_3G;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("cmwap")) {
                    return CMWAP;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("cmnet")) {
                    return CMNET;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("ctwap")) {
                    return CTWAP;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("ctnet")) {
                    return CTNET;
                }
                if (activeNetInfo.getExtraInfo().equalsIgnoreCase("LTE")) {
                    return LTE;
                }
            }
            return MOBILE;
        }
    }

    public static int getISPCode(Context context) {
        TelephonyManager tel = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (tel == null) {
            return NO_SIM_OPERATOR;
        }

        String simOperator = tel.getSimOperator();
        if (simOperator == null || simOperator.length() < 5) { // IMSI
            return NO_SIM_OPERATOR;
        }
        /*
         * http://developer.android.com/reference/android/telephony/TelephonyManager.html#getSimOperator()
         * public String getSimOperator ()
         * Returns the MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits.
         */
        StringBuilder MCC_MNC = new StringBuilder();
        try {
            int len = simOperator.length();
            if (len > 6) {
                len = 6;
            }
            for (int i = 0; i < len; i++) {
                if (!Character.isDigit(simOperator.charAt(i))) {
                    if (MCC_MNC.length() <= 0) { // not begin append
                        continue;
                    } else {
                        break;
                    }
                }
                MCC_MNC.append(simOperator.charAt(i));
            }
            return Integer.valueOf(MCC_MNC.toString());
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
    }

    public static String getISPName(Context context) {
        TelephonyManager tel = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        if (tel == null) {
            return "";
        }

        final int MAX_LENGTH = 100; // ???

        if (tel.getSimOperatorName().length() <= MAX_LENGTH) {
            return tel.getSimOperatorName();
        } else {
            return tel.getSimOperatorName().substring(0, MAX_LENGTH);
        }
    }

    public static int guessNetSpeed(Context context) {
        try {
            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return 100 * 1024;
            }

            switch (netInfo.getSubtype()) {

                case TelephonyManager.NETWORK_TYPE_GPRS:
                    return 4 * 1024;
                case TelephonyManager.NETWORK_TYPE_EDGE:
                    return 8 * 1024;

                case TelephonyManager.NETWORK_TYPE_UMTS:
                case TelephonyManager.NETWORK_TYPE_CDMA:
                case TelephonyManager.NETWORK_TYPE_EVDO_0:
                case TelephonyManager.NETWORK_TYPE_EVDO_A:
                case TelephonyManager.NETWORK_TYPE_1xRTT:
                case TelephonyManager.NETWORK_TYPE_HSDPA:
                case TelephonyManager.NETWORK_TYPE_HSUPA:
                case TelephonyManager.NETWORK_TYPE_HSPA:
                case TelephonyManager.NETWORK_TYPE_IDEN:
                case TelephonyManager.NETWORK_TYPE_EVDO_B:
                case TelephonyManager.NETWORK_TYPE_LTE:
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                    return 100 * 1024;

                case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                default:
                    break;
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return 100 * 1024;
    }

    public static boolean isMobile(Context context) {
        try {
            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            return netInfo.getType() != ConnectivityManager.TYPE_WIFI;

        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;
    }

    public static boolean is2G(Context context) {
        try {
            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return false;
            }
            if (netInfo.getSubtype() == TelephonyManager.NETWORK_TYPE_EDGE || netInfo.getSubtype() == TelephonyManager.NETWORK_TYPE_GPRS || netInfo.getSubtype() == TelephonyManager.NETWORK_TYPE_CDMA) {
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;
    }

    public static boolean is4G(Context context) {
        try {

            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return false;
            }
            // TODO:may be 5G in the future
            if (netInfo.getSubtype() >= TelephonyManager.NETWORK_TYPE_LTE) {
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;

    }

    public static boolean isWap(Context context) {
        int type = getNetType(context);
        return isWap(type);
    }

    public static boolean isWap(int type) {
        return type == UNIWAP || type == CMWAP || type == CTWAP || type == WAP_3G;
    }


    public static boolean is3G(Context context) {

        try {
            final ConnectivityManager manager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            final NetworkInfo netInfo = manager.getActiveNetworkInfo();
            if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return false;
            }
            if (netInfo.getSubtype() >= TelephonyManager.NETWORK_TYPE_EVDO_0 && netInfo.getSubtype() < TelephonyManager.NETWORK_TYPE_LTE) {
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;
    }

    public static boolean isWifi(Context context) {
        int type = getNetType(context);
        return isWifi(type);
    }

    public static boolean isWifi(int type) {
        return type == WIFI;
    }


    public static WifiInfo getWifiInfo(Context context) {
        try {

            ConnectivityManager conMan = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (null == conMan) {
                return null;
            }
            NetworkInfo netInfo = conMan.getActiveNetworkInfo();
            if (null == netInfo || ConnectivityManager.TYPE_WIFI != netInfo.getType()) {
                return null;
            }
            WifiManager wifiMgr = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
            if (null == wifiMgr) {
                return null;
            }
            return wifiMgr.getConnectionInfo();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }


    private static Intent searchIntentByClass(Context context, String className) {
        try {
            PackageManager pmPack = context.getPackageManager();
            List<PackageInfo> packinfos = pmPack.getInstalledPackages(0);
            if (packinfos != null && packinfos.size() > 0) {

                for (int i = 0; i < packinfos.size(); i++) {
                    try {
                        Intent mainIntent = new Intent();
                        mainIntent.setPackage(packinfos.get(i).packageName);
                        List<ResolveInfo> sampleActivityInfos = pmPack.queryIntentActivities(mainIntent, 0);
                        int activityCount = sampleActivityInfos != null ? sampleActivityInfos.size() : 0;
                        if (activityCount > 0) {

                            try {
//								Log.e(TAG, "activityName count " + activityCount);
                                for (int j = 0; j < activityCount; j++) {
                                    ActivityInfo activityInfo = sampleActivityInfos.get(j).activityInfo;
                                    String activityName = activityInfo.name;

                                    if (activityName.contains(className)) {
                                        Intent mIntent = new Intent("/");
                                        ComponentName comp = new ComponentName(activityInfo.packageName, activityInfo.name);
                                        mIntent.setComponent(comp);
                                        mIntent.setAction("android.intent.action.VIEW");
                                        context.startActivity(mIntent);
                                        return mIntent;
                                    }
                                }
                            } catch (Exception eee) {
                                eee.printStackTrace();
                            }

                        }

                    } catch (Exception ee) {
                        ee.printStackTrace();
                    }
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public static void startSettingItent(Context context, int type) {

        switch (type) {
            case TBACKGROUND_NOT_LIMITED: {
                // may not be happen
                break;
            }
            case TBACKGROUND_DATA_LIMITED: {
                try {
                    {
                        Intent mIntent = new Intent("/");
                        ComponentName comp = new ComponentName("com.android.providers.subscribedfeeds", "com.android.settings.ManageAccountsSettings");
                        mIntent.setComponent(comp);
                        mIntent.setAction("android.intent.action.VIEW");
                        context.startActivity(mIntent);
                    }
                } catch (Exception e) {

                    try {
                        Intent mIntent = new Intent("/");
                        ComponentName comp = new ComponentName("com.htc.settings.accountsync", "com.htc.settings.accountsync.ManageAccountsSettings");
                        mIntent.setComponent(comp);
                        mIntent.setAction("android.intent.action.VIEW");
                        context.startActivity(mIntent);
                    } catch (Exception ee) {
                        searchIntentByClass(context, "ManageAccountsSettings");
                    }

                }
                break;
            }
            case TBACKGROUND_PROCESS_LIMITED: {
                try {
                    Intent mIntent = new Intent("/");
                    ComponentName comp = new ComponentName("com.android.settings", "com.android.settings.DevelopmentSettings");
                    mIntent.setComponent(comp);
                    mIntent.setAction("android.intent.action.VIEW");
                    context.startActivity(mIntent);
                } catch (Exception e) {
                    searchIntentByClass(context, "DevelopmentSettings");
                }
                break;
            }
            case TBACKGROUND_WIFI_LIMITED: {
                try {
                    Intent it = new Intent();
                    it.setAction(Settings.ACTION_WIFI_IP_SETTINGS);
                    context.startActivity(it);
                } catch (Exception e) {
                    searchIntentByClass(context, "AdvancedSettings");
                }
                break;
            }
        }
    }

    public static int getWifiSleeepPolicy(Context context) {
        return Settings.System.getInt(context.getContentResolver(), Settings.System.WIFI_SLEEP_POLICY, Settings.System.WIFI_SLEEP_POLICY_NEVER);
    }

    public static boolean isLimited(int type) {
        return type == TBACKGROUND_DATA_LIMITED || type == TBACKGROUND_PROCESS_LIMITED || type == TBACKGROUND_WIFI_LIMITED;
    }

    public static int getBackgroundLimitType(Context context) {
        if (android.os.Build.VERSION.SDK_INT < 14) {
        } else {

            try {
                Class<?> activityManagerNative = Class.forName("android.app.ActivityManagerNative");
                Object am = activityManagerNative.getMethod("getDefault").invoke(activityManagerNative);
                Object limit = am.getClass().getMethod("getProcessLimit").invoke(am);
                if ((Integer) limit == 0) {
                    return TBACKGROUND_PROCESS_LIMITED;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        try {

            int policy = getWifiSleeepPolicy(context);
            if (policy == Settings.System.WIFI_SLEEP_POLICY_NEVER || (getNetType(context) != WIFI)) {
                return TBACKGROUND_NOT_LIMITED;
            } else if (policy == Settings.System.WIFI_SLEEP_POLICY_NEVER_WHILE_PLUGGED || policy == Settings.System.WIFI_SLEEP_POLICY_DEFAULT) {
                return TBACKGROUND_WIFI_LIMITED;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return TBACKGROUND_NOT_LIMITED;
    }

    public static boolean isImmediatelyDestroyActivities(Context context) {
        return Settings.System.getInt(context.getContentResolver(), Settings.System.ALWAYS_FINISH_ACTIVITIES, 0) != 0;
    }

    @SuppressWarnings("deprecation")
    public static int getProxyInfo(Context context, StringBuffer strProxy) {
        try {
            String apiHost = android.net.Proxy.getDefaultHost();
            int apiPort = android.net.Proxy.getDefaultPort();
            if (apiHost != null && apiHost.length() > 0 && apiPort > 0) {
                strProxy.append(apiHost);
                return apiPort;
            }
            String vmHost = System.getProperty("http.proxyHost");
            String vmPort = System.getProperty("http.proxyPort");
            int ivmPort = 80;
            if (vmPort != null && vmPort.length() > 0) {
                ivmPort = Integer.parseInt(vmPort);
            }
            if (vmHost != null && vmHost.length() > 0) {
                strProxy.append(vmHost);
                return ivmPort;
            }
        } catch (final Exception e) {
            e.printStackTrace();
        }

        return 0;
    }

    public static boolean isKnownDirectNet(Context context) {
        int type = getNetType(context);
        return (CMNET == type) || (UNINET == type) || (NET_3G == type) || (CTNET == type) || (LTE == type) || (WIFI == type);
    }

    public static boolean isNetworkConnected(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivityManager == null) {
            return false;
        }
        NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
        if (activeNetInfo == null) {
            return false;
        }

        return activeNetInfo.getState() == NetworkInfo.State.CONNECTED;

    }


    public final static int NETTYPE_NOT_WIFI = 0;
    public final static int NETTYPE_WIFI = 1;

    public final static int UNKNOW_TYPE = 999;

	/*
	 * *
	 * 
	 *  all * 1000
	 * 
	 * 
	 * networktype:TelephonyManager.NETWORK_TYPE_GPRS =  * 1 
	 * networktype:TelephonyManager.NETWORK_TYPE_EDGE = 2 
	 * networktype:TelephonyManager.NETWORK_TYPE_CDMA = 4 
	 * networktype:TelephonyManager.NETWORK_TYPE_EVDO_0 = 5 
	 * networktype:TelephonyManager.NETWORK_TYPE_EVDO_A = 6 
	 * networktype:TelephonyManager.NETWORK_TYPE_EVDO_B = 12
	 * networktype:TelephonyManager.NETWORK_TYPE_HSDPA = 8 
	 * networktype:TelephonyManager.NETWORK_TYPE_UMTS = 3 
	 * networktype:TelephonyManager.NETWORK_TYPE_LTE = 13 
	 * networktype:TelephonyManager.NETWORK_TYPE_IDEN = 11
	 * networktype:TelephonyManager.NETWORK_TYPE_HSUPA = 9 
	 * networktype:TelephonyManager.NETWORK_TYPE_1xRTT = 7 
	 * networktype:TelephonyManager.NETWORK_TYPE_HSPA = 10 
	 * networktype:TelephonyManager.NETWORK_TYPE_EHRPD = 14 
	 * networktype:TelephonyManager.NETWORK_TYPE_HSPAP = 15 
	 * networktype:TelephonyManager.NETWORK_TYPE_UNKNOWN = 0  --> 999
	 */

    private static int nowStrength = 0;

    public static class StrengthListener extends PhoneStateListener {
        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            super.onSignalStrengthsChanged(signalStrength);
            if (!signalStrength.isGsm()) {
                nowStrength = signalStrength.getCdmaDbm();
            } else {
                nowStrength = signalStrength.getGsmSignalStrength();
            }
        }
    }

    public static int getNetTypeForStat(final Context context) {
        if (context == null) {
            return UNKNOW_TYPE;
        }
        try {
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connectivityManager == null) {
                return UNKNOW_TYPE;
            }
            NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
            if (activeNetInfo == null) {
                return UNKNOW_TYPE;
            }
            if (activeNetInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                return NETTYPE_WIFI;
            }
            int subType = activeNetInfo.getSubtype();
            if (subType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                return UNKNOW_TYPE;
            }
            return subType * 1000; // NOTE HERE ~~~
        } catch (Exception e) {
            e.printStackTrace();
        }
        return UNKNOW_TYPE;
    }

    public static int getStrength(final Context context) {
        if (context == null) {
            return 0;
        }
        try {
            if (getNetTypeForStat(context) == NETTYPE_WIFI) {
                return Math.abs(((WifiManager) context.getSystemService(Context.WIFI_SERVICE)).getConnectionInfo().getRssi());
            }
            ((TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE)).listen(new StrengthListener(), PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
            return Math.abs(nowStrength);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return 0;
    }

}
