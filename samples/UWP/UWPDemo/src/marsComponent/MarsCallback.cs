using com.tencent.mars.sample.proto;
using mars;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UWPDemo.model;
using UWPDemo.scene;
using Windows.Storage;

namespace UWPDemo.marsComponent
{

    public class MarsCallback : ICallback_Comm
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

        public Buf2RespRet Buf2Resp(int taskid, int user_context, byte[] inbuffer, int error_code, int channel_select)
        {
            MarsTaskWrapperBase scene = MarsTaskMgr.getTask(taskid);
            if(null == scene)
            {
                return null;
            }

            return scene.onBuf2Resp(inbuffer);
        }

        public void ConsoleLog(int logLevel, string tag, string filename, string funcname, int line, string log)
        {
            string loglevelstring = "";
            switch (logLevel)
            {
                case 0:
                    loglevelstring = "[v]";
                    break;

                case 1:
                    loglevelstring = "[d]";
                    break;
                case 2:
                    loglevelstring = "[i]";
                    break;
                case 3:
                    loglevelstring = "[w]";
                    break;
                case 4:
                    loglevelstring = "[e]";
                    break;
                case 5:
                    loglevelstring = "[f]";
                    break;

                default:
                    break;
            }
            
            string logstring = loglevelstring + filename + "|" + line + "|" + log;
            System.Diagnostics.Debug.WriteLine(logstring);
            return;
        }

        AccountInfoRet accInfo;
        public AccountInfoRet GetAccountInfo()
        {
            if(null == accInfo)
            {
                accInfo = new AccountInfoRet();
                Random rd = new Random();
                accInfo.uin =  (ulong)rd.Next(1000);
                accInfo.username = "anonymous";
            }
            return accInfo;
        }

        public string GetAppFilePath()
        {
            return ApplicationData.Current.LocalFolder.Path;
        }

        public uint GetClientVersion()
        {
            var package = Windows.ApplicationModel.Package.Current;
            uint main = package.Id.Version.Major;
            uint sub = package.Id.Version.Minor;
            uint sp = package.Id.Version.Build;

            uint output = 0;
            output |= (main & 0xff) << 24;
            output |= (sub & 0xff) << 16;
            output |= (sp & 0xff) << 8;
            return output;
        }

        public DeviceInfoRet GetDeviceInfo()
        {
            return new DeviceInfoRet();
        }

        public GetLonglinkIdentifyRet GetLonglinkIdentifyCheckBuffer()
        {
            GetLonglinkIdentifyRet ret = new GetLonglinkIdentifyRet();
            ret.buffer_hash = null;
            ret.nRet = LonglinkCheckType.ECHECK_NEVER;
            return ret;
        }

        public ProxyInfo GetProxyInfo()
        {
            ProxyInfo proxy = new ProxyInfo();
            proxy._host = "";
            proxy.strProxy = "";
            proxy.port = 0;
            return proxy;
        }

        public int GetNetInfo()
        {
            return EWifi;
        }

        public CurRadioAccessNetworkInfo GetCurRadioAccessNetworkInfo()
        {
            CurRadioAccessNetworkInfo info = new CurRadioAccessNetworkInfo();
            info.radio_access_network = "LTE";
            return info;
        }

        public CurWifiInfo GetCurWifiInfo()
        {
            CurWifiInfo info = new CurWifiInfo();
            info.bssid = "no_ssid_wifi";
            info.ssid = "no_ssid_wifi";
            return info;
        }

        public CurSIMInfo GetCurSIMInfo()
        {
            CurSIMInfo info = new CurSIMInfo();
            info.isp_code = "0";
            info.isp_name = "unknown";
            return info;
        }

        public CurAPNInfo GetAPNInfo()
        {
            CurAPNInfo info = new CurAPNInfo();
            info.nettype = 0;
            info.sub_nettype = 0;
            info.extra_info = "";
            return info;
        }

        public uint GetSignal(bool bIsWifi)
        {
            return 1;
        }

        public bool IsNetworkConnected()
        {
            return true;
        }

        public bool IsLogoned()
        {
            return true;
        }

        public bool MakesureAuthed()
        {
            return true;
        }

        public bool OnLonglinkIdentifyResponse(byte[] response_buffer, byte[] identify_buffer_hash)
        {
            return false;
        }

        public string[] OnNewDns(string host)
        {
            // No default new dns support
            string[] iplist = new string[1];
            iplist[0] = "118.89.24.72";
            return iplist;
        }

        public void OnPush(int cmdid, byte[] msgpayload)
        {
            MessagePush msgPush = MessagePush.ParseFrom(msgpayload);
            ChatMsg localMsg = new ChatMsg();
            localMsg.ConversationId = msgPush.Topic;
            localMsg.From = msgPush.From;
            localMsg.IsComMeg = true;
            localMsg.Message = msgPush.Content;
            localMsg.Date = DateTime.Now.ToString();
            MarsEventArgs args = new MarsEventArgs();
            args.Code = EventConst.SUCCESS;
            args.Data = localMsg;
            MarsPushMgr.onPush(cmdid, args);
            return;
        }

        public int OnTaskEnd(int taskid, int user_context, int error_type, int error_code)
        {
            MarsTaskMgr.onTaskEnd(taskid, error_type, error_code);
            return 1;
        }

        public void ReportConnectStatus(int status, int longlink_status)
        {
        }

        public Req2BufRet Req2Buf(int taskid, int user_context, int error_code, int channel_select)
        {
            Req2BufRet ret = new Req2BufRet();
            MarsTaskWrapperBase scene = MarsTaskMgr.getTask(taskid);
            if (null == scene)
            {
                ret.bRet = false;
                ret.nErrCode = -1;
                return ret;
            }

            ret.outbuffer = scene.requestToByteArray();
            ret.bRet = true;
            ret.nErrCode = 0;
            return ret;
        }

        public void RequestSync()
        {
            return;
        }

        public void TrafficData(int _send, int _recv)
        {
            return;
        }
    }
}
