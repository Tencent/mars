#!/usr/bin/env python
# coding:utf-8
# author:dj


from tornado.ioloop import IOLoop
from tornado.iostream import StreamClosedError
from tornado.tcpserver import TCPServer
from tornado.options import options, define

from tornado import ioloop, httpclient, gen
from tornado.gen import Task
import pdb, time, logging
from tornado import stack_context
from tornado.escape import native_str
import sqlite3
import struct

from proto.messagepush_pb2 import MessagePush
from proto.chat_pb2 import SendMessageResponse

define("port_tcp", default=8112, help="TCP port to listen on")
define("web_server_port", default=8209, help="web port")

WEB_SERVER_HOST = '127.0.0.1'

class MarsServer(TCPServer):
    CLIENTVERSION = 200

    CMD_ID_INVALID_VALUE = -1
    CMD_ID_UNKNOWN_VALUE = 0
    CMD_ID_HELLO_VALUE = 1
    CMD_ID_AUTH_VALUE = 2
    CMD_ID_SEND_MESSAGE_VALUE = 3
    CMD_ID_CONVERSATION_LIST_VALUE = 4
    CMD_ID_JOINTOPIC_VALUE = 5

    CMDID_NOOPING = 6
    CMDID_NOOPING_RESP = 6

    CMD_ID_LEFTTOPIC_VALUE = 7
    CMD_ID_HELLO2_VALUE = 8

    MESSAGE_PUSH = 10001

    all_streams = []

    def _get_nooping(self, seq=-1):
        headLength = 4 + 4 + 4 + 4 + 4
        bodyLength = 0
        # packLength = headLength + bodyLength
        clientVersion = self.CLIENTVERSION
        cmdId = self.CMDID_NOOPING_RESP
        seq = seq

        pack = struct.pack(">iiiii", headLength,clientVersion,cmdId,seq,bodyLength)
        return pack

    def _get_hello_resp(self, body='', seq=-1):
        headLength = 4 + 4 + 4 + 4 + 4
        bodyLength = len(body)
        clientVersion = self.CLIENTVERSION
        cmdId = self.CMD_ID_HELLO_VALUE
        seq = seq

        pack = struct.pack(">iiiii", headLength,clientVersion,cmdId,seq,bodyLength)
        return pack

    def _get_msg_resp_header(self, body='', seq=-1):
        headLength = 4 + 4 + 4 + 4 + 4
        bodyLength = len(body)
        clientVersion = self.CLIENTVERSION
        cmdId = self.CMD_ID_SEND_MESSAGE_VALUE
        seq = seq

        pack = struct.pack(">iiiii", headLength,clientVersion,cmdId,seq,bodyLength)
        return pack


    def _get_resp_header(self, body_len=0, seq=-1, cmdId=-1):
        headLength = 4 + 4 + 4 + 4 + 4
        bodyLength = body_len
        # logging.debug("body len:{}".format(bodyLength))
        # logging.debug(seq)

        clientVersion = self.CLIENTVERSION
        cmdId = cmdId
        seq = seq

        pack = struct.pack(">iiiii", headLength,clientVersion,cmdId,seq,bodyLength)
        logging.debug(headLength)
        logging.debug(clientVersion)
        logging.debug(cmdId)
        logging.debug(seq)
        logging.debug(bodyLength)

        # logging.debug(pack)
        return pack

    def _push_message(self, body, seq, stream=None):
        # logging.debug(body)
        sendmsg_resp_m = SendMessageResponse()
        sendmsg_resp_m.ParseFromString(body)

        m = MessagePush()
        m.content = sendmsg_resp_m.text
        setattr(m, 'from', getattr(sendmsg_resp_m, 'from'))
        m.topic = sendmsg_resp_m.topic

        for i in MarsServer.all_streams:
            s = i.get('stream', None)
            # cur_seq = i.get('cur_seq')

            # 黑手 seq == taskid == 0
            header = self._get_resp_header(m.ByteSize(), seq=0, cmdId=self.MESSAGE_PUSH)

            if s != stream:
                # print s
                logging.debug(header)
                logging.debug(m.SerializeToString())
                
                s.write(header)
                # s.write('hahaha')
                logging.debug(m.ByteSize())
                s.write(m.SerializeToString())

    @gen.coroutine
    def handle_stream(self, stream, address):
        logging.debug(stream)
        logging.debug(address)
        MarsServer.all_streams.append({"stream":stream})
        while True:
            try:
                header = yield stream.read_bytes(20)
                headLength, clientVersion, cmdId, seq, bodyLen = struct.unpack('>iiiii', header)
                for i in MarsServer.all_streams:
                    if stream == i.get('stream'):
                        i['cur_seq'] = seq

                logging.debug(" req, dump headLength:{0},clientVersion:{1},cmdId:{2},seq:{3},bodyLen:{4}".format(headLength,clientVersion,cmdId,seq,bodyLen))
                if self.CLIENTVERSION != clientVersion or headLength != 20:
                    logging.error("parsing pack error.")
                    raise Exception
                
                headers = {"Content-Type":"application/octet-stream"}
                # read body
                if bodyLen>0:
                    body = yield stream.read_bytes(bodyLen)
                    if self.CMD_ID_HELLO_VALUE == cmdId:
                        logging.debug("hello msg.")

                        http_client = httpclient.AsyncHTTPClient(
                                                force_instance=True,
                                                defaults=dict(user_agent="mmUserAgent"))
                        response = yield http_client.fetch("http://{0}:{1}/mars/hello".format(WEB_SERVER_HOST, options.web_server_port), method='POST', headers=headers, body = body)

                        resp_header = self._get_hello_resp(body=response.body, seq=seq) 

                        logging.debug('*'*10)
                        logging.debug(resp_header)
                        yield stream.write(resp_header)
                        yield stream.write(response.body)
                    elif self.CMD_ID_SEND_MESSAGE_VALUE == cmdId:
                        logging.debug("send msg.")
                        logging.debug("msg body:{}".format(body))
                        http_client = httpclient.AsyncHTTPClient(force_instance=True,
                                                                defaults=dict(user_agent="mmUserAgent"))
                        response = yield http_client.fetch("http://{0}:{1}/mars/sendmessage".format(WEB_SERVER_HOST, options.web_server_port), method='POST', headers=headers, body = body)
                        resp_header = self._get_msg_resp_header(body=response.body, seq=seq) 

                        self._push_message(response.body, seq, stream)

                        logging.debug(response.body)
                        stream.write(resp_header)
                        stream.write(response.body)
                    
                        logging.debug('sending finish.')

                # no body
                else:
                    if self.CMDID_NOOPING == cmdId:
                        logging.info("nooping.")
                        stream.write(self._get_nooping(seq))
                    elif self.CMD_ID_HELLO2_VALUE == cmdId:
                        logging.debug("hello2.")
                    else:
                        logging.error("cmdID:{}".format(cmdID))
                        # raise Exception

            except StreamClosedError:
                logging.warning("Lost client at host %s", address[0])
                for i in MarsServer.all_streams:
                    if i.get('stream',None) == stream:
                        MarsServer.all_streams.remove(i)
                break
            except Exception as e:
                print(e)


if __name__ == "__main__":
    options.parse_command_line()
    server = MarsServer()
    server.listen(options.port_tcp)
    logging.info("Listening on TCP port %d", options.port_tcp)
    IOLoop.current().start()
