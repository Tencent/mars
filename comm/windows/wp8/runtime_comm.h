/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#pragma once

#include "xlogger.h"
#include "alarm.h"
#include "singleton.h"
#include "platform_comm.h"

#define NETWORK

#ifdef NETWORK
// #include "MMNetCore.h"
//
#endif

// #include "MMLogReport.h"
#include "../../log/loglogic/log_logic.h"
#include "appender.h"


#define KVCOMM

#ifdef KVCOMM
// #include "MMKVCommCore.h"
#endif


namespace PublicComponent {
public ref class Cs2Runtime_Comm sealed {
  public:
    static void OnCreate();
    static void OnDestroy();

    static void OnSingalCrash(int _sig);
    static void OnExceptionCrash();
    static void OnForeground(bool _isforeground);


    static void OnAlarm(int id);
};

public ref struct ProxyInfo sealed {
    property Platform::String^ strProxy;
    property Platform::String^ _host;
    property int port;
};

public ref struct CurWifiInfo sealed {
    property Platform::String^ ssid;
    property Platform::String^ bssid;
};

public ref struct RuntimeNewNetInterfaceInfo sealed {
    property int netType;

    property int ispCode;
    property Platform::String^ ispName;

    property int interfaceType;
    property int interfaceSubType;
    property Platform::String^ interfaceName;
};


public interface class ICallback_Comm {
    bool startAlarm(int id, int after);
    bool stopAlarm(int id);

    int getNetInfo();
    int getStatisticsNetType();

    ProxyInfo^ getProxyInfo();
    bool isNetworkConnected();
    unsigned int getSignal(bool bIsWifi/*isWifi*/);
    CurWifiInfo^ getCurWifiInfo();
    RuntimeNewNetInterfaceInfo^ getNewNetInferfaceInfo();

    void ConsoleLog(int logLevel, Platform::String^ tag, Platform::String^ filename, Platform::String^ funcname, int line, Platform::String^ log);
};

public ref class Runtime2Cs_Comm sealed {
  public:
    static Runtime2Cs_Comm^ Singleton();

    void SetCallback(ICallback_Comm^ _callback);

    bool startAlarm(int id, int after);
    bool stopAlarm(int id);

    int getNetInfo();
    int getStatisticsNetType();

    ProxyInfo^ getProxyInfo();
    bool isNetworkConnected();
    unsigned int getSignal(bool bIsWifi/*isWifi*/);
    CurWifiInfo^ getCurWifiInfo();
    RuntimeNewNetInterfaceInfo^ getNewNetInferfaceInfo();

    void ConsoleLog(int logLevel, Platform::String^ tag, Platform::String^ filename, Platform::String^ funcname, int line, Platform::String^ log);

  private:
    Runtime2Cs_Comm(void);


  private:
    static Runtime2Cs_Comm^ instance;
    ICallback_Comm^ m_callback;
};




}

#endif