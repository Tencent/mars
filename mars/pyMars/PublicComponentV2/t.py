# coding:utf-8
# --author: dj

import mars

def _cb_func(m_str=None, cmd_id=-1, task_id=-1, channel_id=-1):
    from proto.messagepush_pb2 import MessagePush
    if m_str:
        try:
            m = MessagePush()
            m.ParseFromString(m_str.encode('utf-8'))
            print '\n--- cmd_id:{0},task_id:{1} ---'.format(cmd_id, task_id)
            print "topic:{0}".format(m.topic.encode('utf-8'))
            print "content:{0}".format(m.content.encode('utf-8'))
            print "from:{0}".format(getattr(m, 'from').encode('utf-8'))
            print "-" * 30
        except Exception, e:
            print str(e)
            print "parsing error, return origin string to g_str."
            global g_str
            g_str = m_str


def send_test_msg():
    from proto.chat_pb2 import SendMessageRequest
    m = SendMessageRequest()
    setattr(m, 'from', 'py')
    m.to = 'all'
    m.access_token = '123456'
    m.text = 'Hi, from Python 还有中文'
    m.topic = '0'

    mars.sendMsg(m.SerializeToString())


def register_test_push_cb():
    mars.setPushCallback(_cb_func)

# testing...
# if __name__ == '__main__':
#     mars.createMars()
#     register_test_push_cb()
#     send_test_msg()
