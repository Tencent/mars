/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Winhttp.h"
#include <string>
#include <iostream>
#include <map>
#include "alarm.h"
#include "thread/mutex.h"

using namespace std;

#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Wininet.lib")

char* w2c(char* pcstr, size_t len, const wchar_t* pwstr) {
#ifdef WIN32
    int nlength = wcslen(pwstr);
    int nbytes = WideCharToMultiByte(CP_UTF8,   // specify the code page used to perform the conversion
                                     0,  // no special flags to handle unmapped characters
                                     pwstr,  // wide character string to convert
                                     nlength,  // the number of wide characters in that string
                                     NULL,  // no output buffer given, we just want to know how long it needs to be
                                     0,
                                     NULL,  // no replacement character given
                                     NULL);   // we don't want to know if a character didn't make it through the translation

    // make sure the buffer is big enough for this, making it larger if necessary

    if (nbytes > len) nbytes = len;


    WideCharToMultiByte(CP_UTF8,   // specify the code page used to perform the conversion
                        0,  // no special flags to handle unmapped characters
                        pwstr,  // wide character string to convert
                        nlength,  // the number of wide characters in that string
                        pcstr,  // put the output ascii characters at the end of the buffer
                        nbytes,  // there is at least this much space there
                        NULL,  // no replacement character given
                        NULL);

    pcstr[nbytes] = 0;
#else
    sprintf(pcstr, "%s", pwstr);
#endif
    return pcstr ;
}


typedef enum _MX_CS_PROXY_TYPE {
    MX_CS_PROXY_TYPE_NOPROXY = 0,
    MX_CS_PROXY_TYPE_HTTP,
    MX_CS_PROXY_TYPE_SOCKS4,
    MX_CS_PROXY_TYPE_SOCKS4A,
    MX_CS_PROXY_TYPE_SOCKS5,
#ifdef WIN32
    MX_CS_PROXY_TYPE_USEIE,
#endif
} MX_CS_PROXY_TYPE;


bool getProxyAddr(const std::string& strAddr, char* strDestAddr, const char* type) {
    int nStart = strAddr.find(type);

    if (nStart != -1) {
        nStart += strlen(type);
        int nLen = strAddr.find(';', nStart + 1) - nStart;
        strcpy(strDestAddr, strAddr.substr(nStart, nLen).c_str());
        return true;
    }

    return false;
}

int getIEProxy(const wchar_t* host, int& proxytype, int& port, char* strAddr, bool bUserHttps) {
    bool fAutoProxy = false;
    WINHTTP_PROXY_INFO autoProxyInfo = {0};

    WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions = {0};
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = {0};

    if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig)) {
        if (ieProxyConfig.fAutoDetect) {
            fAutoProxy = true;
        }

        if (ieProxyConfig.lpszAutoConfigUrl != NULL) {
            fAutoProxy = true;
            autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
        }
    } else {
        // use autoproxy
        fAutoProxy = true;
    }

    if (fAutoProxy) {
        if (autoProxyOptions.lpszAutoConfigUrl != NULL) {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        } else {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        }

        // basic flags you almost always want
        autoProxyOptions.fAutoLogonIfChallenged = TRUE;

        HINTERNET session = ::WinHttpOpen(0,  // no agent string
                                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                          WINHTTP_NO_PROXY_NAME,
                                          WINHTTP_NO_PROXY_BYPASS,
                                          WINHTTP_FLAG_ASYNC);

        // here we reset fAutoProxy in case an auto-proxy isn't actually
        // configured for this url

        fAutoProxy = WinHttpGetProxyForUrl(session, host, &autoProxyOptions, &autoProxyInfo);

        if (session) WinHttpCloseHandle(session);
    }

    if (fAutoProxy) {
        // set proxy options for libcurl based on autoProxyInfo
        // autoProxyInfo.lpszProxy
        // curl_easy_setopt(curl,CURLOPT_PROXY,autoProxyInfo.lpszProxy);
        if (autoProxyInfo.lpszProxy) {
            w2c(strAddr, 256, autoProxyInfo.lpszProxy);
            proxytype = MX_CS_PROXY_TYPE_HTTP;
            port = 0;
        } else {
            return -1;
        }
    } else {
        if (ieProxyConfig.lpszProxy != NULL) {
            // IE has an explicit proxy. set proxy options for libcurl here
            // based on ieProxyConfig
            //
            // note that sometimes IE gives just a single or double colon
            // for proxy or bypass list, which means "no proxy"
            w2c(strAddr, 256, ieProxyConfig.lpszProxy);
            proxytype = MX_CS_PROXY_TYPE_HTTP;
            port = 0;

            /// may be like this: "http=127.0.0.1:8888;https=127.0.0.1:8888;ftp=127.0.0.1:8888;socks=127.0.0.1:8888" "127.0.0.1:8888"
            string strProxyAddr(strAddr);

            if (strProxyAddr.find('=') != -1) {
                bool bFind = false;

                if (bUserHttps && getProxyAddr(strProxyAddr, strAddr, "https=")) bFind = true;

                if (bFind == false && getProxyAddr(strProxyAddr, strAddr, "http=")) bFind = true;

                if (bFind == false && getProxyAddr(strProxyAddr, strAddr, "socks=")) {
                    proxytype = MX_CS_PROXY_TYPE_SOCKS5;
                }
            }
        } else {
            proxytype = MX_CS_PROXY_TYPE_NOPROXY;
            // there is no auto proxy and no manually configured proxy
        }
    }

    if (autoProxyInfo.lpszProxy != NULL) GlobalFree(autoProxyInfo.lpszProxy);

    if (autoProxyInfo.lpszProxyBypass != NULL) GlobalFree(autoProxyInfo.lpszProxyBypass);

    // if(autoProxyOptions.lpszAutoConfigUrl != NULL) GlobalFree(autoProxyOptions.lpszAutoConfigUrl);
    if (ieProxyConfig.lpszAutoConfigUrl != NULL) GlobalFree(ieProxyConfig.lpszAutoConfigUrl);

    if (ieProxyConfig.lpszProxy != NULL) GlobalFree(ieProxyConfig.lpszProxy);

    if (ieProxyConfig.lpszProxyBypass != NULL) GlobalFree(ieProxyConfig.lpszProxyBypass);

    return proxytype;
}

bool getProxyInfoImpl(int& port, std::string& strProxy, const std::string& _host) {
	std::wstring testHost;
	testHost.assign(_host.begin(), _host.end());

    char strAddr[256] = {0};
    int proxytype = 0;
    getIEProxy(testHost.c_str(), proxytype, port, strAddr, true);

    string strProxyInfo = strAddr;

    if (strProxyInfo.length() == 0) {
        return false;
    }

    int pos = strProxyInfo.find(":");

    if (pos >= 0) {
        strProxy = strProxyInfo.substr(0, pos);
        port = atoi(strProxyInfo.substr(pos + 1, strProxyInfo.length()).c_str());
    }  else {
        strProxy = strProxyInfo;
        port = 0;
    }

	hostent* host = gethostbyname(strProxy.c_str());
	if (host && AF_INET == host->h_addrtype){
		struct in_addr addr = { 0 };
		addr.s_addr = *(u_long *)host->h_addr_list[0];
		strProxy = inet_ntoa(addr);
	}
    return true;
}

DWORD getNetworkStatus() {
    DWORD   flags;  //������ʽ
    BOOL   m_bOnline = InternetGetConnectedState(&flags, 0);
    return flags;
}

bool isNetworkConnectedImpl() {
    DWORD   flags;  //������ʽ

    BOOL   m_bOnline = InternetGetConnectedState(&flags, 0);
    return m_bOnline;

    //if(m_bOnline)//����
    //{
    //    if ((flags & INTERNET_CONNECTION_MODEM) ==INTERNET_CONNECTION_MODEM)
    //    {
    //        cout<<"���ߣ���������\n";
    //    }
    //    if ((flags & INTERNET_CONNECTION_LAN) ==INTERNET_CONNECTION_LAN)
    //    {
    //        cout<<"���ߣ�ͨ��������\n";
    //    }
    //    if ((flags & INTERNET_CONNECTION_PROXY) ==INTERNET_CONNECTION_PROXY)
    //    {
    //        cout<<"���ߣ�����\n";
    //    }
    //    if ((flags & INTERNET_CONNECTION_MODEM_BUSY) ==INTERNET_CONNECTION_MODEM_BUSY)
    //    {
    //        cout<<"MODEM��������INTERNET����ռ��\n";
    //    }
    //}
    // else
    //    cout<<"������\n";
}