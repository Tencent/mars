// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * tickcount.h
 *
 *  Created on: 2014-11-6
 *      Author: yerungui
 */

#ifndef COMM_TICKCOUNT_H_
#define COMM_TICKCOUNT_H_

#include <stdint.h>

class tickcountdiff_t {
  public:
    tickcountdiff_t(int64_t _diff): tickcount_diff_(_diff) {}
    operator int64_t() const {return tickcount_diff_;}

    tickcountdiff_t& operator +=(int64_t _factor)  { tickcount_diff_ += _factor; return *this;}
    tickcountdiff_t& operator -= (int64_t _factor)  { tickcount_diff_ -= _factor; return *this;}
    tickcountdiff_t& operator *=(int64_t _factor)  { tickcount_diff_ *= _factor; return *this;}

  public:
    // tickcountdiff_t(const tickcountdiff_t&);
    // tickcountdiff_t& operator=(const tickcountdiff_t&);

  private:
    int64_t tickcount_diff_;
};

class tickcount_t {
  public:
    tickcount_t(bool _now = false);

    tickcountdiff_t operator-(const tickcount_t& _tc) const { return tickcountdiff_t(tickcount_ - _tc.tickcount_); }

    tickcount_t operator+(const tickcountdiff_t& _tc_diff) const { return tickcount_t(*this).operator += (_tc_diff);}
    tickcount_t operator-(const tickcountdiff_t& _tc_diff) const { return tickcount_t(*this).operator -= (_tc_diff);}

    tickcount_t& operator+=(const tickcountdiff_t& _tc_diff) { tickcount_ += (int64_t)_tc_diff; return *this;}
    tickcount_t& operator-=(const tickcountdiff_t& _tc_diff) { tickcount_ -= (int64_t)_tc_diff; return *this;}

    uint64_t get() const {return tickcount_;}
    tickcount_t& gettickcount();
    tickcountdiff_t gettickspan() const { return tickcount_t(true) - (*this);}

    bool operator< (const tickcount_t& _tc) const { return tickcount_ <  _tc.tickcount_;}
    bool operator<=(const tickcount_t& _tc) const { return tickcount_ <= _tc.tickcount_;}
    bool operator==(const tickcount_t& _tc) const { return tickcount_ == _tc.tickcount_;}
    bool operator!=(const tickcount_t& _tc) const { return tickcount_ != _tc.tickcount_;}
    bool operator> (const tickcount_t& _tc) const { return tickcount_ >  _tc.tickcount_;}
    bool operator>=(const tickcount_t& _tc) const { return tickcount_ >= _tc.tickcount_;}
	bool isValid() { return tickcount_!=0; }

  public:
    // tickcount_t(const tickcount_t&);
    // tickcount_t& operator=(const tickcount_t&);

  private:
    uint64_t tickcount_;
};


#endif /* COMM_TICKCOUNT_H_ */
