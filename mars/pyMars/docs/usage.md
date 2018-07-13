### pyMars 使用说明


 * 使用
    * python cli 使用:
        1. `import mars`
        2. `mars.createMars`
        3. 在 `tests/test_mars.py` 里面有更详细的测试使用方法
        4. 等着消息，或者发消息出去:
            1. 发消息方法: 
                ``` python
                    from proto.chat_pb2 import SendMessageRequest
                    import mars
                    m = SendMessageRequest()
                    setattr(m, 'from', 'py')
                    m.to = 'all'
                    m.access_token = '123456'
                    m.text = '吼吼吼'
                    m.topic = '0'
                    mars.createMars()
                    mars.sendMsg(m.SerializeToString())
                    mars.destroyMars()
                ```
            1. 如何给收到的消息写 callback/handler 的方法: 
                ``` python
                def _cb_func(m_str=None, cmd_id=1, task_id=1, channel_id=1):
                    from proto.messagepush_pb2 import MessagePush
                    if m_str:
                        try:
                            m = MessagePush()
                            m.ParseFromString(m_str.encode('utf8'))
                            print '\n cmd_id:{0},task_id:{1} '.format(cmd_id, task_id)
                            print "topic:{0}".format(m.topic.encode('utf8'))
                            print "content:{0}".format(m.content.encode('utf8'))
                            print "from:{0}".format(getattr(m, 'from').encode('utf8'))
                            print "" * 30
                        except Exception, e:
                            print str(e)
                            print "parsing error, return origin string to g_str."
                            global g_str
                            g_str = m_str
                ```
