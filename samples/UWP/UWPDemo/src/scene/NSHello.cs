using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UWPDemo.scene;
using com.tencent.mars.sample.proto;

namespace UWPDemo.scene
{
    class NSHello : MarsTaskWrapperBaseEx<HelloRequest, HelloResponse, HelloRequest.Builder>
    {
        private string mUser;
        private string mText;

        public NSHello(string user, string text)
        {
            mUser = user;
            mText = text;
        }

        public void doScene()
        {
            beginBuilder();
            mBuilder.User = mUser;
            mBuilder.Text = mText;
            endBuilder();
        }

        protected override string getCgiPath()
        {
            return "/mars/hello";
        }

        protected override int getCmdID()
        {
            return (int)CgiCmdID.CgiCmdID_sayhello;
        }


        protected override int getChannel()
        {
            return (int)CgiChannelType.CgiChannelType_All;
        }

        protected override void onFailed(HelloRequest request, HelloResponse response)
        {
            
        }

        protected override void onFinished(HelloRequest request, HelloResponse response)
        {

        }
    }
}
