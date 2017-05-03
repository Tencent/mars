using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UWPDemo.model;
using UWPDemo.scene;

namespace UWPDemo.ui
{
    public class DataCore : IMarsPushObserver
    {
        private static DataCore mInstance;

        private static readonly object mInsLocker = new object();
        private static readonly object mDataLocker = new object();

        private Dictionary<string, ObservableCollection<ChatMsg>> mDataMap;    //所有数据
        public ObservableCollection<LocalConversation> ConListBinding { get; set; }    //会话数据绑定
        public ObservableCollection<ChatMsg> ChatMsgListBinding { get; set; }      //当前聊天数据绑定
        public string UserName { get; set; }    //用户此次的用户名

        private DataCore()
        {
            mDataMap = new Dictionary<string, ObservableCollection<ChatMsg>>();
            ConListBinding = new ObservableCollection<LocalConversation>();
            ChatMsgListBinding = new ObservableCollection<ChatMsg>();
            MarsPushMgr.addObserver((int)CgiCmdID.CgiCmdID_PushMsg, this);
        }

        public static DataCore getInstance()
        {
            if (mInstance == null)
            {
                lock (mInsLocker)
                {
                    if (mInstance == null)
                    {
                        mInstance = new DataCore();
                    }
                }
            }
            return mInstance;
        }

        public void clearConversationList()
        {
            ConListBinding.Clear(); //不清除会话内数据
        }

        //设置会话列表
        public void setConversationList(ObservableCollection<LocalConversation> conversationList)
        {
            ConListBinding.Clear();

            if (conversationList == null)
            {
                return;
            }

            lock (mDataLocker)
            {
                foreach (LocalConversation conversation in conversationList)
                {
                    if (conversation != null && conversation.ConId != null)
                    {
                        if (!mDataMap.ContainsKey(conversation.ConId))
                        {
                            mDataMap[conversation.ConId] = new ObservableCollection<ChatMsg>();
                        }
                        ConListBinding.Add(conversation);
                    }
                }
            }
        }

        //载入指定会话的消息
        public void loadMsgs(String conversationId)
        {
            if (string.IsNullOrWhiteSpace(conversationId))
                return;

            lock (mDataLocker)
            {
                if (!mDataMap.ContainsKey(conversationId))
                {
                    mDataMap[conversationId] = new ObservableCollection<ChatMsg>();
                }

                ChatMsgListBinding = mDataMap[conversationId];
            }
        }

        //增加一条消息
        public void addMsg(ChatMsg msg)
        {
            if (msg == null)
                return;

            lock (mDataLocker)
            {
                if (!mDataMap.ContainsKey(msg.ConversationId))
                {
                    ObservableCollection<ChatMsg> newList = new ObservableCollection<ChatMsg>();
                    newList.Add(msg);
                    mDataMap[msg.ConversationId] = newList;
                }
                else
                {
                    mDataMap[msg.ConversationId].Add(msg);
                }
            }
        }

        public void onPush(int cmdID, MarsEventArgs args)
        {
            if (cmdID == (int)CgiCmdID.CgiCmdID_PushMsg && args != null && args.Code==EventConst.SUCCESS)
            {
                ChatMsg msg = (ChatMsg) args.Data;
                if(msg != null)
                {
                    addMsg(msg);
                }
            }
        }
    }
}
