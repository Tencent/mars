using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using mars;

namespace UWPDemo.scene
{
    public enum PackResult
    {
        SUCCESS = 0,

        PACK_ERROR,        /// 组包失败
        UNPACK_ERROR,      /// 解包失败 
        RETRY_LIMIT,       /// 重试超出限制
        NET_ERROR,         /// 客户端网络异常
        PACK_TIMEOUT,      /// 包超时（入队列起）
        AUTH_ERROR,        /// 登录失败
        PARSER_ERROR,      /// 解析失败
        BEEN_CANCELLED,     /// 被取消

    }

    public abstract class MarsTaskWrapperBaseEx<TRequest,TResponse, TBuilder> : MarsTaskWrapperBase
        where TRequest : global::Google.ProtocolBuffers.AbstractMessageLite<TRequest, TBuilder>
        where TBuilder : global::Google.ProtocolBuffers.AbstractBuilderLite<TRequest, TBuilder>
    {
		private const string TAG = "NetSceneBaseEx";
   
        protected TBuilder mBuilder;
        protected PackResult mErrorCode = PackResult.SUCCESS;
        protected TRequest mReqObj;
        protected TResponse mResObj;
        protected TaskRuntime mTaskRuntime = new TaskRuntime();

        /// <summary>
        ///  Builder 初始化
        /// </summary>
        protected bool beginBuilder()
        {
            try
            {
                Type type = typeof(TRequest);
                TypeInfo tinfo = type.GetTypeInfo();
                IEnumerable<MethodInfo>  methods = tinfo.GetDeclaredMethods("CreateBuilder");
                foreach (MethodInfo mt in methods)
                {
                    ParameterInfo[]  paras = mt.GetParameters();
                    if (paras.Length == 0)
                    {
                        mBuilder = mt.Invoke(null, null) as TBuilder;
                        break;
                    }
                }

                if (mBuilder != null)
                {
                    return true;
                }
            }
            catch (Exception e)
            {
            }
            Debug.Assert(false, TAG);
            return false;
        }


        /// <summary>
        ///  结束builder,组包发送
        /// </summary>
        protected bool endBuilder()
        {
            mReqObj = mBuilder.Build();

            mTaskRuntime.taskid = mTaskId;
            mTaskRuntime.cgi = getCgiPath();
            mTaskRuntime.cmdid = getCmdID();
            mTaskRuntime.channel_select = getChannel();
            string[] hostList = new string[1];
            hostList[0] = getHost();
            mTaskRuntime.shortlink_host_list= hostList;
               
            MarsTaskMgr.doTask(this);
            
            mBuilder = null;
            
            return true;
        }
        
        public override byte[] requestToByteArray()
        {
            return mReqObj.ToByteArray();
        }
        
        public override TaskRuntime getTask()
        {
            return mTaskRuntime;
        }

        public override Buf2RespRet onBuf2Resp(byte[] inbuffer)
        {
            Buf2RespRet ret = new Buf2RespRet();
            ret.bRet = -1;
            try
            {
                Type type = typeof(TResponse);
                object[] args = new object[1];
                args[0] = inbuffer;

                mResObj = (TResponse)type.GetTypeInfo().GetDeclaredMethod("ParseFrom").Invoke(null, args);
                ret.bRet = 0;
            }
            catch (System.Reflection.TargetInvocationException ex)
            {

            }
            catch (Exception e)
            {

            }
            return ret;
        }


        public override void onCompleted(int error_type, int error_code)
        {
            if(error_type != 0 || error_code != 0 || mResObj == null)
            {
                onFailed(mReqObj, mResObj);
            }
            else
            {
                onFinished(mReqObj, mResObj);
            }
        }
        
        protected PackResult getLastError()
        {
            return mErrorCode;
        }

        virtual protected void onSampleSpeed(int RTT)
        {
        }
     
        /// <summary>
        /// 客户端错误，如发送超时或网络错误等
        /// </summary>
        /// <returns></returns>
        abstract protected void onFailed(TRequest request, TResponse response);

        /// <summary>
        /// 服务器返回
        /// </summary>
        /// <returns></returns>
        abstract protected void onFinished(TRequest request, TResponse response);

        public bool check(string args)
        {
            return (args != null) && (args.Length > 0);
        }
    }
}