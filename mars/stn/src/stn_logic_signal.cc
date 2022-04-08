//
// Created by Cpan on 2022/3/10.
//

#include "stn_logic_signal.h"


boost::signals2::signal<void ()>& GetStnLogicSignalOnCreate()
{
  static boost::signals2::signal<void ()> StnLogicSignalOnCreate;
  return StnLogicSignalOnCreate;
}


boost::signals2::signal<void (int _encoder_version)>& GetStnLogicSignalOnInitBeforeOnCreate() {

  static boost::signals2::signal<void (int _encoder_version)> StnLogicSignalOnInitBeforeOnCreate;
  return StnLogicSignalOnInitBeforeOnCreate;
}

boost::signals2::signal<void ()>& GetStnLogicSignalOnDestroy()
{
  static boost::signals2::signal<void ()> StnLogicSignalOnDestroy;
  return StnLogicSignalOnDestroy;
}

boost::signals2::signal<void (int _sig)>& GetStnLogicSignalOnSingalCrash()
{
  static boost::signals2::signal<void (int _sig)> StnLogicSignalOnSingalCrash;
  return StnLogicSignalOnSingalCrash;
}

boost::signals2::signal<void ()>& GetStnLogicSignalOnExceptionCrash()
{
  static boost::signals2::signal<void ()> StnLogicSignalOnExceptionCrash;
  return StnLogicSignalOnExceptionCrash;
}

boost::signals2::signal<void (bool _isForeground)>& GetStnLogicSignalOnForeground()
{
  static boost::signals2::signal<void (bool _isForeground)> StnLogicSignalOnForeground;
  return StnLogicSignalOnForeground;
}

boost::signals2::signal<void ()>& GetStnLogicSignalOnNetworkChange()
{
  static boost::signals2::signal<void ()> StnLogicSignalOnNetworkChange;
  return StnLogicSignalOnNetworkChange;
}


boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetStnLogicSignalOnNetworkDataChange() {
  static boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)> StnLogicSignalOnNetworkDataChange;
  return StnLogicSignalOnNetworkDataChange;
}

boost::signals2::signal<void (int64_t _id)>& GetStnLogicSignalOnAlarm() {
  static boost::signals2::signal<void (int64_t _id)> StnLogicSignalOnAlarm;
  return StnLogicSignalOnAlarm;
}