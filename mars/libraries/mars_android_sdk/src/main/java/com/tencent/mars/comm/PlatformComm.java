package com.tencent.mars.comm;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.net.Proxy;
import android.os.Handler;
import android.telephony.TelephonyManager;
import android.util.Log;

/**
 * mars获取
 * Created by caoshaokun on 16/3/7.
 */
public class PlatformComm {

	private static final String TAG = "PlatformComm";

	private final static boolean IS_PROXY_ON = false;


	public static Context context = null;
	public static Handler handler = null;

    static final int ENoNet = -1;
	static final int EWifi = 1;
	static final int EMobile = 2;
	static final int EOtherNet = 3;

	static final int NETTYPE_NOT_WIFI = 0;
	static final int NETTYPE_WIFI = 1;
	static final int NETTYPE_WAP = 2;
	static final int NETTYPE_2G = 3;
	static final int NETTYPE_3G = 4;
	static final int NETTYPE_4G = 5;
	static final int NETTYPE_UNKNOWN = 6;
	static final int NETTYPE_NON = -1;

	/**
	 * WiFi信息类
	 */
    static class WifiInfo {
    	public String ssid;
    	public String bssid;
    }

    /**
	 * 手机卡信息类
	 */
    static class SIMInfo {
    	public String ispCode;
    	public String ispName;
    }

    /**
	 * 接入点信息
	 */
    static class APNInfo {
		public int netType;
		public int subNetType;
		public String extraInfo;
	}

	public static void init(Context ncontext, Handler nhandler) {
		context = ncontext;
		handler = nhandler;

		NetworkSignalUtil.InitNetworkSignalUtil(ncontext);
	}

	public static class C2Java {

		/**
		 * mars回调获取网络类型
		 * @return WiFi/Mobile/NoNet
         */
		public static int getNetInfo() {
			ConnectivityManager conMan = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

			if (null == conMan) {
				return ENoNet;
			}

			NetworkInfo netInfo = conMan.getActiveNetworkInfo();

			if (null == netInfo) {
				return ENoNet;
			}

			try {
				switch (netInfo.getType()) {
				case ConnectivityManager.TYPE_WIFI:
					return EWifi;
				case ConnectivityManager.TYPE_MOBILE:
				case ConnectivityManager.TYPE_MOBILE_DUN:
				case ConnectivityManager.TYPE_MOBILE_HIPRI:
				case ConnectivityManager.TYPE_MOBILE_MMS:
				case ConnectivityManager.TYPE_MOBILE_SUPL:
					return EMobile;
				default:
					return EOtherNet;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			return EOtherNet;
		}

		/**
		 * mars回调获取Http代理信息
		 * @param strProxy
         * @return
         */
		public static int getProxyInfo(StringBuffer strProxy) {

			if (!IS_PROXY_ON) {
				return -1;
			}

			int proxyPort = -1;
			String proxy = "";
			try {
				proxy = Proxy.getDefaultHost();
				proxyPort = Proxy.getDefaultPort();
				if (proxy != null && proxy.length() > 0 && proxyPort > 0) {
					strProxy.append(proxy);
					return proxyPort;
				}
				proxy = System.getProperty("http.proxyHost");
				String vmPort = System.getProperty("http.proxyPort");
				if (vmPort != null && vmPort.length() > 0) {
					proxyPort = Integer.parseInt(vmPort);
				}
				if (proxy != null && proxy.length() > 0) {
					strProxy.append(proxy);
					return proxyPort;
				}
			} catch (final Exception e) {
				e.printStackTrace();
			}

			strProxy.append(proxy);
			return proxyPort;



		}

	    public static int getStatisticsNetType() {
	        if (null==context){
	            return 0;
	        }

	        try {
	    		int ret = NetStatusUtil.getNetType(context);
	    		if (ret == NetStatusUtil.NON_NETWORK) {
	    			return NETTYPE_NON;
	    		}
	    		if (NetStatusUtil.is2G(context)) {
	    			return NETTYPE_2G;
	    		} else if (NetStatusUtil.is3G(context)) {
	    			return NETTYPE_3G;
	    		} else if (NetStatusUtil.is4G(context)) {
	    			return NETTYPE_4G;
	    		} else if (NetStatusUtil.isWifi(ret)) {
	    			return NETTYPE_WIFI;
	    		} else if (NetStatusUtil.isWap(ret)) {
	    			return NETTYPE_WAP;
	    		} else {
	    			return NETTYPE_UNKNOWN;
	    		}
	        }
	        catch (Exception e) {
				e.printStackTrace();
	        }

	        return NETTYPE_NON;
	    }

		/**
		 * 启动定时器
		 * @param id
		 * @param after
         * @return
         */
	    public static boolean startAlarm(final int id, final int after) {
	        if (null==context){
	            return false;
	        }

	        try {
	            return Alarm.start(id, after, context);
	        }
	        catch (Exception e) {
	            e.printStackTrace();
	        }
	        return false;

	    }

		/**
		 * 停止定时器
		 * @param id
		 * @return
         */
	    public static boolean stopAlarm(final int id) {
	        if (null==context){
	            return false;
	         }

	        try {
	            return Alarm.stop(id, context);
	        }
	        catch (Exception e) {
	            e.printStackTrace();
	        }
	        return false;
	    }

		/**
		 * 获取当前WiFi的具体信息
		 * @return
         */
	    public static WifiInfo getCurWifiInfo() {
	    	try {
	    		if (null == context) return null;

	    		ConnectivityManager conMan = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

	    		if(null == conMan)	return null;

	    		NetworkInfo netInfo = conMan.getActiveNetworkInfo();
	    		if (null == netInfo || ConnectivityManager.TYPE_WIFI != netInfo.getType()) {
	    			return null;
	    		}

	    		WifiManager wifiMgr = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
	    		if (null == wifiMgr)	return null;

	    		android.net.wifi.WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
	    		if (null == wifiInfo) return null;

	    		WifiInfo info = new WifiInfo();
	    		info.ssid = wifiInfo.getSSID();
	    		info.bssid = wifiInfo.getBSSID();
	    		return info;
	    	} catch (Exception e) {
	    		e.printStackTrace();
	    	}
	    	return null;
	    }

		/**
		 * 获取当前手机卡信息
		 * @return
         */
	    public static SIMInfo getCurSIMInfo() {
	    	try {

	    		if (null == context) return null;

	    		int ispCode = NetStatusUtil.getISPCode(context);
	    		if (NetStatusUtil.NO_SIM_OPERATOR == ispCode) {
	    			return null;
	    		}

	    		SIMInfo simInfo = new SIMInfo();
	    		simInfo.ispCode = "" + ispCode;
	    		simInfo.ispName = NetStatusUtil.getISPName(context);
	    		return simInfo;
	    	} catch (Exception e) {
	    		e.printStackTrace();
	    	}

	    	return null;
	    }

		/**
		 * 获取接入点信息
		 * @return
         */
	    public static APNInfo getAPNInfo() {
			try {
				final ConnectivityManager manager = (ConnectivityManager) context
						.getSystemService(Context.CONNECTIVITY_SERVICE);
				final NetworkInfo netInfo = manager.getActiveNetworkInfo();
				APNInfo apnInfo = new APNInfo();
				if (netInfo != null) {
					apnInfo.netType = netInfo.getType();
					apnInfo.subNetType = netInfo.getSubtype();
					if (ConnectivityManager.TYPE_WIFI != netInfo.getType()) {
						apnInfo.extraInfo = (netInfo.getExtraInfo() == null) ? "" : netInfo.getExtraInfo();
					} else {
						WifiInfo wifiInfo = getCurWifiInfo();
						apnInfo.extraInfo = wifiInfo.ssid;
					}
					return apnInfo;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}

			return null;
		}

		public static int getCurRadioAccessNetworkInfo(){
			if (null==context){
				return TelephonyManager.NETWORK_TYPE_UNKNOWN;
			}

			try {

				TelephonyManager telephonyManager = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
				return telephonyManager.getNetworkType();
			}
			catch (Exception e) {
				e.printStackTrace();
			}

			return TelephonyManager.NETWORK_TYPE_UNKNOWN;
		}

		/**
		 * mars回调获取网络信号强度
		 * @param isWifi
         * @return
         */
	    public static long getSignal(boolean isWifi) {

	 	   try {
	 	       if (null==context){
	 	           return 0;
	 	       }

	 		   if(isWifi)
				return NetworkSignalUtil.getWifiSignalStrength();
	 		   else
				return NetworkSignalUtil.getGSMSignalStrength();
	 	   } catch(Exception e) {
			   e.printStackTrace();
	 		   return 0;
	 	   }
	    }

		/**
		 * mars回调查看终端网络是否已连接状态
		 * @return
         */
	    public static boolean isNetworkConnected() {
	        if (null==context){
	            return false;
	        }

	        try {
	            return NetStatusUtil.isNetworkConnected(context);
	        }
	        catch (Exception e) {
				e.printStackTrace();
	        }

	        return false;
	    }

	    public static WakerLock wakeupLock_new() {
            if (null == context) {
                return null;
            }

            try {
                return new WakerLock(context);
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
	}
}
