/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * anti_avalanche.cc
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#include "anti_avalanche.h"

#include "mars/baseevent/active_logic.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/singleton.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"

#include "flow_limit.h"
#include "frequency_limit.h"

using namespace mars::stn;

AntiAvalanche::AntiAvalanche(bool _isactive)
	: frequency_limit_(new FrequencyLimit())
	, flow_limit_(new FlowLimit((_isactive)))
{}

AntiAvalanche::~AntiAvalanche() {
    delete flow_limit_;
    delete frequency_limit_;
}

bool AntiAvalanche::Check(const Task& _task, const void* _buffer, int _len) {
    xverbose_function();

    unsigned int span = 0;
    if (!frequency_limit_->Check(_task, _buffer, _len, span)){
		ReportTaskLimited(kFrequencyLimit, _task, span);
    	return false;
    }

    if (kMobile == getNetInfo() && !flow_limit_->Check(_task, _buffer, _len)) {
    	ReportTaskLimited(kFlowLimit, _task, (unsigned int&)_len);
		return false;
    }

    return true;
}

void AntiAvalanche::OnSignalActive(bool _isactive) {
    flow_limit_->Active(_isactive);
}
