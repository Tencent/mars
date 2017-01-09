using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PublicComponent;
using Microsoft.Phone.Net.NetworkInformation;
using Microsoft.Phone.Storage;
using System.IO.IsolatedStorage;
using System.IO;
using System.Reflection;
using System.Diagnostics;



namespace PlatformComm
{

    public class CCallbackImpl_Comm : ICallback_Comm
    {

        static readonly int ENoNet = -1;
        static readonly int EWifi = 1;
        static readonly int EMobile = 2;
        static readonly int EOther = 3;

        static readonly int NETTYPE_NON = -1;
        static readonly int NETTYPE_NOT_WIFI = 0;
        static readonly int NETTYPE_WIFI = 1;
        static readonly int NETTYPE_WAP = 2;
        static readonly int NETTYPE_2G = 3;
        static readonly int NETTYPE_3G = 4;
        static readonly int NETTYPE_4G = 5;


        public int getStatisticsNetType()
        {
            NetworkInterfaceList interfaceList = new NetworkInterfaceList();
            NetworkInterfaceInfo interfaceInfo = interfaceList.First();

            switch (interfaceInfo.InterfaceSubtype)
            { 
                case NetworkInterfaceSubType.Cellular_3G:
                case NetworkInterfaceSubType.Cellular_EVDO:
                case NetworkInterfaceSubType.Cellular_EVDV:
                case NetworkInterfaceSubType.Cellular_HSPA:
                    return NETTYPE_3G;
                case NetworkInterfaceSubType.Cellular_GPRS:
                case NetworkInterfaceSubType.Cellular_EDGE:
                case NetworkInterfaceSubType.Cellular_1XRTT:
                    return NETTYPE_2G;
                case NetworkInterfaceSubType.Cellular_EHRPD:
                case NetworkInterfaceSubType.Cellular_LTE:
                    return NETTYPE_4G;
                case NetworkInterfaceSubType.WiFi:
                    return NETTYPE_WIFI;
                default:
                    return NETTYPE_NON;

            
            }
        }


        public int getNetInfo()
        {
            NetworkInterfaceList interfaceList = new NetworkInterfaceList();
            NetworkInterfaceInfo interfaceInfo = interfaceList.First();

            Debug.WriteLine("NetInfo:" + interfaceInfo.InterfaceType);
            switch (interfaceInfo.InterfaceType)
            {
                case NetworkInterfaceType.None:
                    return ENoNet;
                case NetworkInterfaceType.Wireless80211:
                    return EWifi;
                case NetworkInterfaceType.MobileBroadbandCdma:
                case NetworkInterfaceType.MobileBroadbandGsm:
                    return EMobile;
                default:
                    return EOther;
            }

        }

        public bool startAlarm(int id, int after)
        {
            return Alarm.start(id, after);
        }

        public bool stopAlarm(int id)
        {
            return Alarm.stop(id);
        }


        public bool isNetworkConnected()
        {
            NetworkInterfaceList interfaceList = new NetworkInterfaceList();
            NetworkInterfaceInfo interfaceInfo = interfaceList.First();

            if (interfaceInfo.InterfaceState == ConnectState.Connected)
                return true;
            else if (interfaceInfo.InterfaceState == ConnectState.Disconnected)
                return false;
            else
                return false;
        }




        public CurWifiInfo getCurWifiInfo()
        {
            CurWifiInfo info = new CurWifiInfo();
            NetworkInterfaceList interfaceList = new NetworkInterfaceList();
            NetworkInterfaceInfo interfaceInfo = interfaceList.First();
            if (!DeviceNetworkInformation.IsWiFiEnabled) return null;

            info.ssid = interfaceInfo.InterfaceName;

            return info;
        }

        public ProxyInfo getProxyInfo()
        {
           
            return null;
        }

        public void ConsoleLog(int logLevel, string tag, string filename, string funcname, int line, string log)
        {
            string logstr = DateTime.Now.ToLongTimeString();
            //string logstr = "";
            switch (logLevel)
            { 
                case 0:
                    logstr += " [logLevel]:V";
                    break;
                case 1:
                    logstr += " [logLevel]:D";
                    break;
                case 2:
                    logstr += " [logLevel]:I";
                    break;
                case 3:
                    logstr += " [logLevel]:W";
                    break;
                case 4:
                    logstr += " [logLevel]:E";
                    break;
                default:
                    logstr += " [logLevel]:N";
                    break;

            }
           
            logstr+= " [tag]:" + tag;
            //logstr+= " [filename]:" + filename;
            logstr+= " [funcname]:" + filename;
            logstr+= " [line]:" + line;
            logstr+= " [log]:" + log;

            Debug.WriteLine(logstr);
           
        }
    }


}
