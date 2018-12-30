using mars;
using System.Diagnostics;

public enum CgiChannelType
{
    CgiChannelType_ShortConn =1,
    CgiChannelType_LongConn=2,
    CgiChannelType_All =3,
}

public enum CgiCmdID
{
    CgiCmdID_sayhello =1,
    CgiCmdID_ConvList = 2,
    CgiCmdID_SendMsg = 3,
    CgiCmdID_PushMsg = 10001,
}

namespace UWPDemo.scene
{

    public interface INetSceneControl
    {
        bool doCancel();
    }
	public abstract class MarsTaskWrapperBase
	{
		private const string TAG = "NetSceneBase";
        
        static int gTaskId = 1;
        public int mTaskId = gTaskId++;

        private static uint     acc_uin = 0;
        private static byte[]   acc_SessionKey = new byte[36];

        public static void onLogin(uint id , byte[] sessionkey)
        {
            Debug.Assert(sessionkey != null, "sessionkey != null");

            acc_uin = id;
            acc_SessionKey = sessionkey;
        }
        
        public abstract TaskRuntime getTask();

        public abstract byte[] requestToByteArray();

        public abstract Buf2RespRet onBuf2Resp(byte[] inbuffer);

        protected abstract string getCgiPath();

        protected abstract int getCmdID();

        protected abstract int getChannel();

        protected virtual string getHost()
        {
            return "www.marsopen.cn";
        }
        
        public abstract void onCompleted(int error_type, int error_code);
        
        public void cancel()
        {
            //not implement
        }
        
	}
}