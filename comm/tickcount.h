/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
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
