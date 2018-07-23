# coding:utf-8
# author: dj

import mars
import fire

from proto.messagepush_pb2 import MessagePush
from proto.chat_pb2 import SendMessageRequest

class mm_Client(object):
    """A pyMars maimai client class."""

    def __init__(self, mars=None):
        super(mm_Client, self).__init__()    
        if None == mars:
            import mars
        self._mars = mars
        self._connected = 0

    def _cb_func(self, m_str=None, cmd_id=-1, task_id=-1, channel_id=-1):
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

    def connect(self):
        if self._connected == 0:
            print 'connecting.'
            self._mars.createMars(self._cb_func)
            self._connected = 1
        else:
            print 'connected.'

    def send_msg(self, msg='Hi, from Python 还有中文', \
                    to='all', origin='py', cmd_type=-1, topic='0'):
        if self._connected == 0:
            self.connect()
        
        m = SendMessageRequest()
        setattr(m, 'from', origin)
        m.to = to
        m.access_token = '123456'
        m.text = msg
        m.topic = topic

        self._mars.sendMsg(m.SerializeToString())

if __name__ == '__main__':
  fire.Fire(mm_Client)