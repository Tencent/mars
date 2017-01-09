package com.tencent.mars.comm;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;

public class NetworkSignalUtil {
	private static long strength = 10000;
	private static Context context = null;
	public static final String TAG = "MicroMsg.NetworkSignalUtil";
	
	public static void InitNetworkSignalUtil(Context ncontext) {
		context = ncontext;
		TelephonyManager mgr = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
		mgr.listen(new PhoneStateListener() {
			@Override
			public void onSignalStrengthsChanged(SignalStrength signalStrength){
				super.onSignalStrengthsChanged(signalStrength);
				calSignalStrength(signalStrength);
			}
		} ,PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
	}
	
	public static long getNetworkSignalStrength(boolean isWifi) {
		return 0;
	}
	
	public static long getGSMSignalStrength() {
		return strength;
	}
	
	public static long getWifiSignalStrength() {
		WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
		WifiInfo info = wifiManager.getConnectionInfo();
		if (info != null && info.getBSSID() != null) {
			//calculateSignalLevel param 2 must < 46, otherwise divide by 0 exception will happen
			int sig = WifiManager.calculateSignalLevel(info.getRssi(), 10);
			sig = (sig > 10 ? 10 : sig);
			sig = (sig < 0 ? 0 : sig);
			return sig * 10;
		}
		return 0;
	}

	private static void calSignalStrength(SignalStrength sig) {
		int nSig = 0;
		if(sig.isGsm())
			nSig = sig.getGsmSignalStrength();
		else
			nSig = (sig.getCdmaDbm() + 113) / 2; 
		if(sig.isGsm() && nSig == 99)
			strength = 0;
		else {
			strength = (long)(nSig * ((float)100 / (float)31));
			strength = (strength > 100 ? 100 : strength);
			strength = (strength < 0 ? 0 : strength);
		}
	}
}
