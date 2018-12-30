using com.tencent.mars.sample.proto;
using UWPDemo.model;
using System.Collections.ObjectModel;

namespace UWPDemo.scene
{
    public class NSConvList : MarsTaskWrapperBaseEx<ConversationListRequest,
                            ConversationListResponse, ConversationListRequest.Builder>
    {
        private string mToken;
        private int mType;

        public NSConvList(int type, string token)
        {
            mType = type;
            mToken = token;
        }

        public void doScene()
        {
            beginBuilder();
            mBuilder.Type = mType;
            mBuilder.AccessToken = mToken;
            endBuilder();
        }

        protected override string getCgiPath()
        {
            return "/mars/getconvlist";
        }

        protected override int getCmdID()
        {
            return (int)CgiCmdID.CgiCmdID_ConvList;
        }

        protected override int getChannel()
        {
            return (int)CgiChannelType.CgiChannelType_ShortConn;
        }

        protected override void onFailed(ConversationListRequest request, ConversationListResponse response)
        {
            MarsEventArgs args = new MarsEventArgs();
            args.Code = EventConst.FAIL;
            MarsPushMgr.onPush( getCmdID(), args);
        }

        protected override void onFinished(ConversationListRequest request, ConversationListResponse response)
        {
            MarsEventArgs args = new MarsEventArgs();
            if (response == null )
            {
                args.Code = EventConst.FAIL;
            }
            else
            {
                args.Code = EventConst.SUCCESS;
                ObservableCollection<LocalConversation> resultList = new ObservableCollection<LocalConversation>();
                if (response.ListList != null)
                {
                    foreach (Conversation con in response.ListList)
                    {
                        LocalConversation localCon = new LocalConversation();
                        localCon.ConId = con.Topic;
                        localCon.ConName = con.Name;
                        localCon.ConNotice = con.Notice;
                        resultList.Add(localCon);
                    }
                }
                args.Data = resultList;
            }
            MarsPushMgr.onPush(getCmdID(), args);
        }
    }
}
