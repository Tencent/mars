/**
 * created on : 2012-11-28
 * author : yerungui
 */
#include "comm/platform_comm.h"

#include "comm/xlogger/xlogger.h"
#include "comm/xlogger/loginfo_extract.h"
#import "comm/objc/scope_autoreleasepool.h"

#import <TargetConditionals.h>
#if !TARGET_OS_WATCH
#import <SystemConfiguration/CaptiveNetwork.h>
#import <CFNetwork/CFProxySupport.h>
#endif

#if TARGET_OS_IPHONE && !TARGET_OS_WATCH
#import <UIKit/UIApplication.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <CoreTelephony/CTCarrier.h>
#import <NetworkExtension/NetworkExtension.h>
#endif

#if !TARGET_OS_IPHONE
#import <CoreWLAN/CWInterface.h>
#import <CoreWLAN/CWWiFiClient.h> 
#endif

#include <MacTypes.h>

#include "comm/objc/objc_timer.h"
#import "comm/objc/Reachability.h"

#include "comm/thread/lock.h"
#include "comm/network/getifaddrs.h"

#if !TARGET_OS_IPHONE
static float __GetSystemVersion() {
    //	float system_version = [UIDevice currentDevice].systemVersion.floatValue;
    //	return system_version;
    NSString *versionString;
    NSDictionary * sv = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    if (nil != sv){
        versionString = [sv objectForKey:@"ProductVersion"];
        return versionString != nil ? versionString.floatValue : 0;
    }
    
    return 0;
}
#endif

static MarsNetworkStatus __GetNetworkStatus()
{
#if TARGET_OS_WATCH
    return ReachableViaWiFi;
#else
    return [MarsReachability getCacheReachabilityStatus:NO];
#endif
}

void FlushReachability() {
   [MarsReachability getCacheReachabilityStatus:YES];
}

float publiccomponent_GetSystemVersion() {
    //	float system_version = [UIDevice currentDevice].systemVersion.floatValue;
    //	return system_version;
    NSString *versionString;
    NSDictionary * sv = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    if (nil != sv){
        versionString = [sv objectForKey:@"ProductVersion"];
        return versionString != nil ? versionString.floatValue : 0;
    }
    
    return 0;
}

bool getProxyInfo(int& port, std::string& strProxy, const std::string& _host)
{
    xverbose_function();
    
#if TARGET_OS_WATCH
    return false;
    
#else
    SCOPE_POOL();
    NSDictionary *proxySettings = [NSMakeCollectable((NSDictionary *)CFNetworkCopySystemProxySettings()) autorelease];
    if (nil==proxySettings) return false;
    
    CFURLRef url = (CFURLRef)[NSURL URLWithString: [NSString stringWithUTF8String:(_host.c_str())]];
    
//    CFURLRef url = (CFURLRef)[NSURL URLWithString: @"http://www.google.com"];
    
    
    NSArray *proxies = [NSMakeCollectable((NSArray *)CFNetworkCopyProxiesForURL(url, (CFDictionaryRef)proxySettings)) autorelease];
    
    if (nil==proxies || 0==[proxies count]) return false;

    NSDictionary *settings = [proxies objectAtIndex:0];
    CFStringRef http_proxy = (CFStringRef)[settings objectForKey:(NSString *)kCFProxyHostNameKey];
    CFNumberRef http_port = (CFNumberRef)[settings objectForKey:(NSString *)kCFProxyPortNumberKey];
    CFStringRef http_type = (CFStringRef)[settings objectForKey:(NSString *)kCFProxyTypeKey];
    
   if (0==CFStringCompare(http_type, kCFProxyTypeAutoConfigurationURL, 0))
   {
       CFErrorRef  error = nil;
       CFStringRef proxyAutoConfigurationScript = (CFStringRef)[settings objectForKey:(NSString *)kCFProxyAutoConfigurationURLKey];
       if (nil==proxyAutoConfigurationScript) return false;
       
       proxies = [NSMakeCollectable((NSArray *)CFNetworkCopyProxiesForAutoConfigurationScript(proxyAutoConfigurationScript, url, &error)) autorelease];
       
       if (nil!=error) return false;

       
       if (nil==proxies || 0==[proxies count]) return false;
       settings = [proxies objectAtIndex:0];
       
       http_proxy = (CFStringRef)[settings objectForKey:(NSString *)kCFProxyHostNameKey];
       http_port = (CFNumberRef)[settings objectForKey:(NSString *)kCFProxyPortNumberKey];
   }
    
    if (nil==http_proxy || nil==http_port) return false;
    
    char tmp_proxy[128] = {0};
    int tmp_port = 0;
    

    if (!CFStringGetCString(http_proxy, tmp_proxy, sizeof(tmp_proxy), kCFStringEncodingASCII)
        || !CFNumberGetValue(http_port, kCFNumberSInt32Type, &tmp_port))
    {
        xerror2(TSF"convert error");
        return false;
    }

    strProxy = tmp_proxy;
    port = tmp_port;
    xdebug2(TSF"%0:%1", strProxy, port);
    return true;
#endif
}

int getNetInfo() {
    xverbose_function();
    SCOPE_POOL();
    
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_WATCH
    return kWifi;
#endif

    switch (__GetNetworkStatus())
    {
        case NotReachable:
            return kNoNet;
        case ReachableViaWiFi:
            return kWifi;
        case ReachableViaWWAN:
            return kMobile;
        default:
            return kNoNet;
    }
}

unsigned int getSignal(bool isWifi){
    xverbose_function();
    SCOPE_POOL();
    return (unsigned int)0;
}




void ConsoleLog(const XLoggerInfo* _info, const char* _log)
{
    SCOPE_POOL();

    if (NULL==_info || NULL==_log) return;
    
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"  // fatal
    };
    
    char strFuncName[128]  = {0};
    ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
    
    const char* file_name = ExtractFileName(_info->filename);
    
    char log[16 * 1024] = {0};
    snprintf(log, sizeof(log), "[%s][%s][%s, %s, %d][%s", levelStrings[_info->level], NULL == _info->tag ? "" : _info->tag, file_name, strFuncName, _info->line, _log);
    
    
    NSLog(@"%@", [NSString stringWithUTF8String:log]);
}

bool isNetworkConnected()
{
   SCOPE_POOL(); 
    switch (__GetNetworkStatus())
    {
        case NotReachable:
            return false;
        case ReachableViaWiFi:
            return true;
        case ReachableViaWWAN:
return true;
        default:
            return false;
    }
}

#define SIMULATOR_NET_INFO "SIMULATOR"
#define IWATCH_NET_INFO "IWATCH"
#define USE_WIRED  "wired"

bool getCurWifiInfo(WifiInfo& wifiInfo)
{
    SCOPE_POOL();
    
#if TARGET_IPHONE_SIMULATOR
    wifiInfo.ssid = SIMULATOR_NET_INFO;
    wifiInfo.bssid = SIMULATOR_NET_INFO;
    return true;
#elif !TARGET_OS_IPHONE
    
    static CWInterface* info = nil; //CWInterface can reused
    
    if (nil == info) {
        if (__GetSystemVersion() < 10.10){
            info = [CWInterface interface];
        }else{
            CWWiFiClient* wificlient = [CWWiFiClient sharedWiFiClient];
            if (nil != wificlient) info = [wificlient interface];
        }
    }

    if (nil == info) return false;
    if (info.ssid) {
        wifiInfo.ssid = [info.ssid UTF8String];
        //wifiInfo.bssid = [info.bssid UTF8String];
    } else {
        wifiInfo.ssid = USE_WIRED;
        wifiInfo.bssid = USE_WIRED;
    }
    return true;
    
#elif TARGET_OS_WATCH
    wifiInfo.ssid = IWATCH_NET_INFO;
    wifiInfo.bssid = IWATCH_NET_INFO;
    return true;
#else
    static Mutex mutex;
    NSArray *ifs = nil;
    {
        ScopedLock lock(mutex);
        ifs = (id)CNCopySupportedInterfaces();
    }

    if (ifs == nil) return false;
        
    id info = nil;
    for (NSString *ifnam in ifs) {
        info = (id)CNCopyCurrentNetworkInfo((CFStringRef)ifnam);
        if (info && [info count] && info[@"SSID"]) {
            break;
        }
            
        if (nil!=info) {
            CFRelease(info);
            info = nil;
        }
    }
        
    if (info == nil) {
        CFRelease(ifs);
        return false;
    }
        
    const char* ssid_cstr = [[info objectForKey:@"SSID"] UTF8String];
    const char* bssid_cstr = [[info objectForKey:@"BSSID"] UTF8String];
    if (NULL != ssid_cstr) {
        wifiInfo.ssid = ssid_cstr;
    }
        
    if (NULL != bssid_cstr) {
        wifiInfo.bssid = bssid_cstr;
    }
    CFRelease(info);
    CFRelease(ifs);
    
    return true;
#endif
}

#if TARGET_OS_IPHONE && !TARGET_OS_WATCH
bool getCurSIMInfo(SIMInfo& simInfo)
{
    static Mutex mutex;
    ScopedLock lock(mutex);
    
    SCOPE_POOL();
    static CTTelephonyNetworkInfo* s_networkinfo = [[CTTelephonyNetworkInfo alloc] init];
    CTCarrier* carrier = s_networkinfo.subscriberCellularProvider;
    
    if (nil == carrier)
    {
        return false;
    }
    
    if (nil == carrier.mobileCountryCode || nil == carrier.mobileNetworkCode)
    {
        return false;
    }

    simInfo.isp_code += [carrier.mobileCountryCode UTF8String];
    simInfo.isp_code += [carrier.mobileNetworkCode UTF8String];
    xverbose2(TSF"isp_code:%0", simInfo.isp_code);

    return true;
}

#else

bool getCurSIMInfo(SIMInfo& simInfo)
{
    return false;
}
#endif

bool getAPNInfo(APNInfo& info)
{
    RadioAccessNetworkInfo raninfo;
    if (kMobile != getNetInfo()) return false;
    if (!getCurRadioAccessNetworkInfo(raninfo)) return false;
    
    info.nettype = kMobile;
    info.extra_info = raninfo.radio_access_network;
    return true;
}

bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip)
{
    std::vector<ifaddrinfo_ipv4_t> addrs;
    if (!getifaddrs_ipv4_lan(addrs)) return false;
    
    for (auto it=addrs.begin(); it!=addrs.end(); ++it)
    {
        if (std::string::npos != it->ifa_name.find("bridge"))
        {
            _ifname = it->ifa_name;
            _ifip = it->ip;
            return true;
        }
    }
    
    return false;
}

/**
CTTelephonyNetworkInfo *telephonyInfo = [CTTelephonyNetworkInfo new];
NSLog(@"Current Radio Access Technology: %@", telephonyInfo.currentRadioAccessTechnology);
[NSNotificationCenter.defaultCenter addObserverForName:CTRadioAccessTechnologyDidChangeNotification 
                                                object:nil 
                                                 queue:nil 
                                            usingBlock:^(NSNotification *note) 
{
    NSLog(@"New Radio Access Technology: %@", telephonyInfo.currentRadioAccessTechnology);
}];
**/

#if TARGET_OS_IPHONE && !TARGET_OS_WATCH
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _raninfo)
{
    SCOPE_POOL();
    
    if (!([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0)) { return false;}
    static CTTelephonyNetworkInfo* s_networkinfo = [[CTTelephonyNetworkInfo alloc] init];
    
    NSString *currentRadioAccessTechnology = s_networkinfo.currentRadioAccessTechnology;
    if (!currentRadioAccessTechnology) { return false;}
    
    _raninfo.radio_access_network =  [currentRadioAccessTechnology UTF8String];
    _raninfo.radio_access_network.erase(0, strlen("CTRadioAccessTechnology"));
    
    xassert2(!_raninfo.radio_access_network.empty(), "%s", [currentRadioAccessTechnology UTF8String]);
    
    return true;
}
#else
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _raninfo)
{
   return false;
}
#endif

void comm_export_symbols_1(){}

