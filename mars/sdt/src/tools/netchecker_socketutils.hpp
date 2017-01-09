//
//  netchecker_socketutils.hpp
//  netchecker
//
//  Created by renlibin on 2/7/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#ifndef NETCHECKER_SRC_NETCHECKER_SOCKETUTILS_HPP_
#define NETCHECKER_SRC_NETCHECKER_SOCKETUTILS_HPP_

#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>

#include "mars/comm/socket/unix_socket.h"
#else
#include <WS2tcpip.h>
#endif // !_WIN32

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/time_utils.h"
#include "mars/stn/config.h"

#include "sdt.h"
#include "constants.h"

using namespace mars::sdt;

class NetCheckerSocketUtils {
    
public:
    /*
	 * param: timeoutInMs if set 0, then select timeout param is NULL, not timeval(0)
	 * return value:
	 * -1 --- error
	 * -2 --- timeout
	 * -3 --- PIPE INTR
	 * -4 --- PIPE select error
	 * >=0 --- suc
	 */
	static SOCKET makeNonBlockSocket(SocketSelect& sel, const std::string& strIp, int port, unsigned int timeoutInMs, int& errCode)
	{
		xverbose_function();
        xdebug2(TSF"makeNonBlockSocket, ip: %0, port: %1", strIp, port);
		struct sockaddr_in _addr;
		bzero(&_addr, sizeof(_addr));
		_addr.sin_family = AF_INET;
		_addr.sin_addr.s_addr =inet_addr(strIp.c_str());
		_addr.sin_port = (unsigned short int)htons(port);
        
		SOCKET fsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fsocket == INVALID_SOCKET) {
			errCode = socket_errno;
			xerror2(TSF"socket create error, socket_errno:%0", socket_strerror(errCode));
			return INVALID_SOCKET;
		}
        
        if (0 != socket_ipv6only(fsocket, 0)){
            xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno));
        }
		//set the socket to unblocked model
		int ret = socket_set_nobio(fsocket);
		if (ret != 0) {
			errCode = socket_errno;
			xerror2(TSF"nobio:%0", socket_strerror(errCode));
			::socket_close(fsocket);
			return INVALID_SOCKET;
		}
        
		//do connect
		unsigned long lastTime = gettickcount();
		int connectRet = connect(fsocket, (sockaddr*)&_addr, sizeof(_addr));
		if (connectRet < 0 && !IS_NOBLOCK_CONNECT_ERRNO(socket_errno))
		{
			errCode = socket_errno;
			xerror2(TSF"connect error, socket_errno:%0", socket_strerror(errCode));
			::socket_close(fsocket);
			return INVALID_SOCKET;
		}
        
		if (connectRet != 0)
		{
            int intrCount = 0;
			while(true)
			{
				sel.PreSelect();
				sel.Write_FD_SET(fsocket);
				sel.Exception_FD_SET(fsocket);
                
	            int selectRet = -1;
				if (timeoutInMs > 0)
				{
		            selectRet = sel.Select(timeoutInMs);
				}
				else
					selectRet = sel.Select();
                
				if (selectRet == 0) {
					errCode = socket_errno;
					xerror2(TSF"connect timeout, use time:%0 ms to connect", (gettickcount() - lastTime));
					::socket_close(fsocket);
					return -2;
				} else if (selectRet < 0) {
					errCode = socket_errno;
					xerror2(TSF"select errror, ret:%0, socket_errno:%1, use time:%2 ms",
                           selectRet, socket_strerror(errCode), (gettickcount() - lastTime));
                    
					if (selectRet == -1 && EINTR == errCode && intrCount < 3)
					{
						intrCount += 1;
						continue;
					}
					::socket_close(fsocket);
					return INVALID_SOCKET;
				}
				if (sel.IsException())
				{
					errCode = errno;
					xerror2(TSF"select breaker exception");
					::socket_close(fsocket);
					return INVALID_SOCKET;
				}
				if (sel.IsBreak())
				{
					xinfo2(TSF"Breaker INTR");
					::socket_close(fsocket);
					return INVALID_SOCKET;
				}
				if (sel.Exception_FD_ISSET(fsocket))
				{
					int error = 0;
					socklen_t len = sizeof(error);
					ret = getsockopt(fsocket, SOL_SOCKET, SO_ERROR, &error, &len);
					if (ret == 0)
					{
						errCode = error;
						xerror2(TSF"select socket exception error:%0", strerror(errCode));
					}
					else
						errCode = errno;
					xerror2(TSF"select socket exception");
                    
					::socket_close(fsocket);
					return INVALID_SOCKET;
				}
				if (!sel.Write_FD_ISSET(fsocket))
				{
					errCode = errno;
					xerror2(TSF"select return but not set, return:%0, errno:%1", selectRet, errno);
					::socket_close(fsocket);
					xassert2(false);
					return INVALID_SOCKET;
				}
                
				xdebug2(TSF"use time:%0 ms to connect", (gettickcount() - lastTime));
                
				sockaddr addr;
				socklen_t len = sizeof(addr);
				memset(&addr, 0, sizeof(sockaddr));
				int ret = getpeername(fsocket, &addr, &len);
				if (ret != 0) {
					errCode = socket_errno;
					int error = 0;
					len = sizeof(error);
					ret = getsockopt(fsocket, SOL_SOCKET, SO_ERROR, &error, &len);
					if (ret == 0)
					{
						errCode = error;
						xerror2(TSF"connect error:%0", socket_strerror(errCode));
					}
					else
						xerror2(TSF"getsockopt error");
					::socket_close(fsocket);
					return INVALID_SOCKET;
				}
                
				xdebug2(TSF"connect success");
				break;
			}
		}
        
		return fsocket;
	}
    
	/*
	 * return value:
	 *              0  --- suc
     *          others --- error
	 */
	static TcpErrCode writenWithNonBlock(int fdSocket, SocketSelect& sel, unsigned int timeoutMs, const unsigned char* buff, unsigned int unSize, int &errcode)
	{
		xverbose_function();
        xinfo2(TSF"writenWithNonBlock with Socket:%0, timeoutMs:%1, unSize:%2", fdSocket, timeoutMs, unSize);
		xassert2(unSize > 0);
		if (unSize == 0)
		{
			xwarn2(TSF"writen size == 0");
			return kTcpSucc;
		}
        
		unsigned long costMs = 0;
		unsigned int nSent = 0;
		int intrCount = 0;
		while(true)
		{
			unsigned int remainLen = unSize - nSent;
			int ret = -1;
			xassert2(remainLen > 0);
            
			sel.PreSelect();
			sel.Write_FD_SET(fdSocket);
			sel.Exception_FD_SET(fdSocket);
            
			unsigned long startMs = gettickcount();
			if (timeoutMs > 0)
			{
				if (costMs >= timeoutMs)
				{
					errcode = -1;
					return kTimeoutErr;
				}
				else
				{
					ret = sel.Select(timeoutMs);
				}
			}
			else
				ret = sel.Select();
            
			if (ret == -1)
			{
				errcode = errno;
				xerror2(TSF"select return -1, error:%0", strerror(errcode));
                if (EINTR == errcode && intrCount < 3)
                {
                	intrCount += 1;
                	continue;
                }
				return kSelectErr;
			}
			if (ret == 0)
			{
				xerror2(TSF"select timeout");
				errcode = -1;
				return kTimeoutErr;
			}
			if (sel.IsException())
			{
				xerror2(TSF"select pipe error");
				errcode = errno;
				return kPipeExp;
			}
			if (sel.IsBreak())
			{
				xwarn2(TSF"INTR by pipe");
				return kPipeIntr;
			}
			if (sel.Exception_FD_ISSET(fdSocket))
			{
				int error = 0;
				socklen_t len = sizeof(error);
				ret = getsockopt(fdSocket, SOL_SOCKET, SO_ERROR, &error, &len);
				if (ret == 0)
				{
					errcode = error;
					xerror2(TSF"select socket exception error:%0", strerror(errcode));
				}
				else
					errcode = errno;
				xerror2(TSF"select socket exception");
                
				return kSelectExpErr;
			}
			if (sel.Write_FD_ISSET(fdSocket))
			{
				ssize_t nwrite =::send(fdSocket, buff, remainLen, 0);
				errcode = errno;		//never do other things after send, otherwise errno will change
				xdebug2(TSF"sendWithNonBlock ::send return:%0", nwrite);
				if (nwrite == 0 || (0 > nwrite && !IS_NOBLOCK_SEND_ERRNO(socket_errno)))
				{
					xerror2(TSF"sendWithNonBlock send <= 0, errno:%0", strerror(errcode));
					return kSndRcvErr;
				}
                
				if (0 > nwrite) nwrite = 0;
                
				buff += nwrite;
				nSent += nwrite;
                
				if (nSent >= unSize)
					return kTcpSucc;
                
				costMs += (gettickcount() - startMs);
				continue;
			}
            
			xerror2(TSF"select return but none is select");
			xassert2(false);
			return kAssertErr;
		}
		return kTcpSucc;
	}
    
	/*
	 * Des:recv unSize buffer or recv until socket close
	 *
	 * return value:
	 *              0  ------ suc and unsize recv
	 *              1  ------ suc and socket close
     *          others ------ error
	 */
	static TcpErrCode readnWithNonBlock(int fdSocket, SocketSelect& sel, unsigned int timeoutMs, AutoBuffer& recvBuf, unsigned int unSize, int &errcode)
	{
	    xverbose_function();
		xdebug2(TSF"readnWithNonBlock socket:%0, timeoutMs:%1", fdSocket, timeoutMs);
		if (unSize == 0)
			return kTcpSucc;
        if (timeoutMs == 0) timeoutMs = DEFAULT_TCP_RECV_TIMEOUT;
		if ((recvBuf.Capacity() - recvBuf.Length()) < unSize)
		{
			recvBuf.AddCapacity(unSize);
		}
		recvBuf.Seek(0, AutoBuffer::ESeekEnd);
		size_t oldLength = recvBuf.Length();
        
		unsigned long costMs = 0;
        int intrCount = 0;
		while(true)
		{
			size_t length = unSize + oldLength - recvBuf.Length();
			int ret = -1;
            
			sel.PreSelect();
			sel.Read_FD_SET(fdSocket);
			sel.Exception_FD_SET(fdSocket);
            
			unsigned long startMs = gettickcount();
			if (timeoutMs > 0)
			{
				if (costMs >= timeoutMs)
				{
					errcode = -1;
					return kTimeoutErr;
				}
				else
				{
					ret = sel.Select(timeoutMs);
				}
			}
			else
				ret = sel.Select();
			if (ret == -1)
			{
				errcode = errno;
				xerror2(TSF"select return -1, error:%0", strerror(errcode));
				if (EINTR == errcode && intrCount < 3)
				{
					intrCount += 1;
					continue;
				}
				return kSelectErr;
			}
			if (ret == 0)
			{
				xerror2(TSF"select timeout");
				errcode = -1;
				return kTimeoutErr;
			}
			if (sel.IsException())
			{
				xerror2(TSF"select pipe exception");
				errcode = errno;
				return kPipeExp;
			}
			if (sel.IsBreak())
			{
				xwarn2(TSF"INTR by pipe");
				return kPipeIntr;
			}
			if (sel.Exception_FD_ISSET(fdSocket))
			{
				int error = 0;
				socklen_t len = sizeof(error);
				ret = getsockopt(fdSocket, SOL_SOCKET, SO_ERROR, &error, &len);
				if (ret == 0)
				{
					errcode = error;
					xerror2(TSF"select socket exception error:%0", strerror(errcode));
				}
				else
					errcode = errno;
				xerror2(TSF"select socket exception");
                
				return kSelectExpErr;
			}
			if (sel.Read_FD_ISSET(fdSocket))
			{
				ssize_t nrecv = ::recv(fdSocket, recvBuf.PosPtr(), length, 0);
				errcode = errno;			//never do other things after recv, otherwise errno will be changed
				xdebug2(TSF"readnWithNonBlock recv :%0", nrecv);
				if (nrecv < 0)
				{
					xerror2(TSF"readnWithNonBlock readn nrecv < 0, errno:%0", strerror(errcode));
					return kSelectErr;
				}
				else if (nrecv == 0)
				{
					xinfo2(TSF"nrecv==0, socket close:%0", errno);
					return kTcpNonErr;
				}
                
				recvBuf.Length(nrecv + recvBuf.Pos(), nrecv + recvBuf.Pos());
                
				if ((recvBuf.Length() - oldLength) >= unSize)
				{
					xdebug2(TSF"recvBuf.Length()=%0, oldLength=%1, unSize=%2",recvBuf.Length(), oldLength, unSize);
					return kTcpNonErr;
				}
			}
			costMs += (gettickcount() - startMs);
		}
		return kTcpSucc;
	}
    
};

#endif	// NETCHECKER_SRC_NETCHECKER_SOCKETUTILS_HPP_
