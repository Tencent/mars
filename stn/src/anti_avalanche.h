/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * anti_avalanche.h
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#ifndef STN_SRC_ANTI_AVALANCHE_H_
#define STN_SRC_ANTI_AVALANCHE_H_

namespace mars {
namespace stn {

struct Task;

class FrequencyLimit;
class FlowLimit;

enum {
	kFrequencyLimit = 1,
	kFlowLimit = 2
};

class AntiAvalanche {
  public:
    AntiAvalanche(bool _isactive);
    virtual ~AntiAvalanche();

    bool Check(const Task& _task, const void* _buffer, int _len);
    void OnSignalActive(bool _isactive);

  public:
    AntiAvalanche(const AntiAvalanche&);
    AntiAvalanche& operator=(const AntiAvalanche&);

  private:
    FrequencyLimit* frequency_limit_;
    FlowLimit* flow_limit_;
};

}}

#endif // STN_SRC_ANTI_AVALANCHE_H_
