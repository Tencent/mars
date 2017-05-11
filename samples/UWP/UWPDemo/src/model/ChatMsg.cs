using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UWPDemo.model
{
    public class ChatMsg
    {
        public string From { get; set; }    //消息来自

        public string ConversationId { get; set; }    //会话id

        public string Date { get; set; }    //消息日期

        public string Message { get; set; }    //消息内容

        public bool IsComMeg { get; set; }   // 是否为收到的消息

    }
}
