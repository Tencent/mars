using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using com.tencent.mars.sample.proto;

namespace UWPDemo.scene
{
    class NSSendmsg : MarsTaskWrapperBaseEx<SendMessageRequest, SendMessageResponse, SendMessageRequest.Builder>
    {
        private string mToken;
        private string mFrom;
        private string mTopic;
        private string mText;

        public NSSendmsg(string token, string from, string topic, string text)
        {
            mToken = token;
            mFrom = from;
            mTopic = topic;
            mText = text;
        }

        public void doScene()
        {
            beginBuilder();
            mBuilder.From = mFrom;
            mBuilder.To = "all";
            mBuilder.Text = mText;
            mBuilder.AccessToken = mToken;
            mBuilder.Topic = mTopic;
            endBuilder();
        }

        protected override string getCgiPath()
        {
            return "/mars/sendmessage";
        }

        protected override int getCmdID()
        {
            return (int)CgiCmdID.CgiCmdID_SendMsg;
        }

        protected override int getChannel()
        {
            return (int)CgiChannelType.CgiChannelType_LongConn;
        }

        protected override void onFailed(SendMessageRequest request, SendMessageResponse response)
        {

        }

        protected override void onFinished(SendMessageRequest request, SendMessageResponse response)
        {

        }
    
    }
}
