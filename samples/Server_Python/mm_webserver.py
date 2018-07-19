#!/usr/bin/env python
# coding:utf-8
# author:dj
#
# Copyright 2009 Facebook
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

import logging
import tornado.escape
import tornado.ioloop
import tornado.web
import os.path

from tornado.concurrent import Future
from tornado import gen
from tornado.options import define, options, parse_command_line

from mm_tornado_protobuff import ProtocolBufferMixin
from proto.main_pb2 import HelloRequest, HelloResponse
from proto.main_pb2 import ConversationListRequest, Conversation, ConversationListResponse
from proto.chat_pb2 import SendMessageRequest, SendMessageResponse


define("port", default=8209, help="run on the given port", type=int)
define("debug", default=True, help="run in debug mode")


class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("5L2g5aW977yM6ISJ6ISJ5Zyo5oub5Lq677yM5pyJ5YW06Laj6K+36IGU57O75oiRLg==")


class HelloHandler(tornado.web.RequestHandler, ProtocolBufferMixin):
    @gen.coroutine
    def get(self):
        self.write('5L2g5aW977yM6ISJ6ISJ5Zyo5oub5Lq677yM5pyJ5YW06Laj6K+36IGU57O75oiRLg==')

    @gen.coroutine
    def post(self):
        try:
            print 'post...'
            print self.request.body
            m = self.read_protobuf(HelloRequest, self.request.body)
            print m
            resp_str = 'congratulations, ' + m.user
            print resp_str
            m2 = HelloResponse()
            m2.retcode = 0
            m2.errmsg = resp_str
            self.write_protobuf(m2, format='application/octet-stream')
        except:
            self.write('format error.')

class GetConversationListHandler(tornado.web.RequestHandler, ProtocolBufferMixin):
    @gen.coroutine
    def get(self):
        self.write("get conversation list handler.")

    @gen.coroutine
    def post(self):
        try:
            m = self.read_protobuf(ConversationListRequest, self.request.body)
            print "request from access_token=%s, type=%d" % (m.access_token, m.type)

            c1 = Conversation()
            c1.topic = "0"
            c1.name = "Mars"
            c1.notice = "STN Discuss"

            c2 = Conversation()
            c2.topic = "1"
            c2.name = "Mars"
            c2.notice = "Xlog Discuss"

            c3 = Conversation()
            c3.topic = "2"
            c3.name = "Mars"
            c3.notice = "SDT Discuss"

            cl = ConversationListResponse()
            cl.list.extend((c1,c2,c3))
            print cl
            self.write_protobuf(cl, format='application/octet-stream')
        except:
            print "conv list error."



class SendMessageHandler(tornado.web.RequestHandler, ProtocolBufferMixin):
    @gen.coroutine
    def get(self):
        self.write("send msg handler.")

    @gen.coroutine
    def post(self):
        try:
            m = self.read_protobuf(SendMessageRequest, self.request.body)
            print "request from user=%s, text=%s to topic=%s" % (getattr(m,'from'), m.text, m.topic)

            err_msg = "congratulations, " + getattr(m, 'from')
            
            resp_m = SendMessageResponse()
            resp_m.err_code = 0
            resp_m.err_msg = err_msg
            setattr(resp_m, 'from', getattr(m, 'from'))
            resp_m.text = m.text
            resp_m.topic = m.topic

            self.write_protobuf(resp_m, format='application/octet-stream')
        except:
            print "send msg error."


def main():
    parse_command_line()
    app = tornado.web.Application(
        [
            (r"/", MainHandler),
            (r"/mars/hello", HelloHandler),
            (r"/mars/getconvlist", GetConversationListHandler),
            (r"/mars/sendmessage", SendMessageHandler),
        ],
        cookie_secret="__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__",
        # TODO: 看下对其他地方有没有影响
        xsrf_cookies=False,
        debug=options.debug,
    )
    app.listen(options.port)
    tornado.ioloop.IOLoop.current().start()


if __name__ == "__main__":
    print "dj webserver starting."
    main()
