/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * TcpFSMHandler.h
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#ifndef TcpFSMHandler_H_
#define TcpFSMHandler_H_

#include "comm/xlogger/xlogger.h"

template<class InputIterator>
bool TcpFSMHandler(InputIterator _first, InputIterator _last, SocketSelectBreaker& _breaker, bool _select_anyway)
{
    xgroup2_define(group);

    SocketSelect sel(_breaker, true);
    sel.PreSelect();

    bool have_runing_status = false;
    int timeout = INT_MAX;

    for (InputIterator it = _first; it != _last; ++it)
    {
        if (!(*it)->IsEndStatus()) have_runing_status = true;

        timeout = std::min(timeout, (*it)->Timeout());
        (*it)->PreSelect(sel, group);
    }

    if (!have_runing_status && !_select_anyway)
    {
        xinfo2(TSF"all end status") >> group;
        return false;
    }

    int ret = 0;
    if (INT_MAX == timeout)
    {
        ret = sel.Select();
    } else {
        timeout = std::max(0, timeout);
        ret = sel.Select(timeout);
    }

    // select error
    if (ret < 0) { xerror2(TSF"sel err ret:(%_, %_)", ret, sel.Errno()) >> group; return false;}
    // user break
    if (sel.IsException()) { xerror2(TSF"breaker exp") >> group; return false; }
    if (sel.IsBreak()) { xdebug2(TSF"breaker break") >> group; return false; }

    for (InputIterator it = _first; it != _last; ++it)
    {
        (*it)->AfterSelect(sel, group);
    }

    return true;
}

template<class InputIterator>
void TcpFSMHandlerRunloop(InputIterator _first, InputIterator _last, SocketSelectBreaker& _breaker, bool _select_anyway)
{
    xinfo_function();

     while (TcpFSMHandler(_first, _last, _breaker, _select_anyway)) {}
}

#endif
