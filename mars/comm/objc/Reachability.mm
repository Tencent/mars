/*
 
 File: Reachability.m
 Abstract: Basic demonstration of how to use the SystemConfiguration Reachablity APIs.
 
 Version: 2.2
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under
 Apple's copyrights in this original Apple software (the "Apple Software"), to
 use, reproduce, modify and redistribute the Apple Software, with or without
 modifications, in source and/or binary forms; provided that if you redistribute
 the Apple Software in its entirety and without modifications, you must retain
 this notice and the following text and disclaimers in all such redistributions
 of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may be used
 to endorse or promote products derived from the Apple Software without specific
 prior written permission from Apple.  Except as expressly stated in this notice,
 no other rights or licenses, express or implied, are granted by Apple herein,
 including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be
 incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
 DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
 CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
 APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2010 Apple Inc. All Rights Reserved.
 
*/

#import <sys/socket.h>
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>

#import <CoreFoundation/CoreFoundation.h>

#import "comm/objc/Reachability.h"


#if TARGET_OS_WATCH
//nothing
#else

#define kShouldPrintReachabilityFlags 0

static MarsReachability* gs_MarsnetReach = [[MarsReachability reachabilityForInternetConnection] retain];
static BOOL gs_Marsstartnotify = [gs_MarsnetReach MarsstartNotifier];
static MarsNetworkStatus gs_Marsstatus = [gs_MarsnetReach currentReachabilityStatus];

static void PrintReachabilityFlags(SCNetworkReachabilityFlags    flags, const char* comment)
{
#if kShouldPrintReachabilityFlags
#if TARGET_OS_IPHONE    
    NSLog(@"Reachability Flag Status: %c%c %c%c%c%c%c%c%c %s\n",
            (flags & kSCNetworkReachabilityFlagsIsWWAN)                  ? 'W' : '-',
#else
    NSLog(@"Reachability Flag Status: %c %c%c%c%c%c%c%c %s\n",
#endif
            (flags & kSCNetworkReachabilityFlagsReachable)            ? 'R' : '-',
            
            (flags & kSCNetworkReachabilityFlagsTransientConnection)  ? 't' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionRequired)   ? 'c' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)  ? 'C' : '-',
            (flags & kSCNetworkReachabilityFlagsInterventionRequired) ? 'i' : '-',
            (flags & kSCNetworkReachabilityFlagsConnectionOnDemand)   ? 'D' : '-',
            (flags & kSCNetworkReachabilityFlagsIsLocalAddress)       ? 'l' : '-',
            (flags & kSCNetworkReachabilityFlagsIsDirect)             ? 'd' : '-',
            comment
            );
#endif
}
          
@implementation MarsReachability
static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info)
{
    #pragma unused (target, flags)
    NSCAssert(info != NULL, @"info was NULL in ReachabilityCallback");
    NSCAssert([(NSObject*) info isKindOfClass: [MarsReachability class]], @"info was wrong class in ReachabilityCallback");

    //We're on the main RunLoop, so an NSAutoreleasePool is not necessary, but is added defensively
    // in case someon uses the Reachablity object in a different thread.
    NSAutoreleasePool* myPool = [[NSAutoreleasePool alloc] init];
    
    MarsReachability* noteObject = (MarsReachability*) info;
    gs_Marsstatus = [noteObject currentReachabilityStatus];
    
    // Post a notification to notify the client that the network reachability changed.
    [[NSNotificationCenter defaultCenter] postNotificationName: kReachabilityChangedNotification object: noteObject];
    
    [myPool release];
}

          
- (BOOL) MarsstartNotifier
{
    BOOL retVal = NO;
    SCNetworkReachabilityContext    context = {0, self, NULL, NULL, NULL};
    if(SCNetworkReachabilitySetCallback(reachabilityRef, ReachabilityCallback, &context))
    {
        //use CFRunLoopGetMain instead of CFRunLoopGetCurrent, because threads created by pthread do not have runloop
        if(SCNetworkReachabilityScheduleWithRunLoop(reachabilityRef, CFRunLoopGetMain(), kCFRunLoopDefaultMode))
        {
            retVal = YES;
        }
    }
    return retVal;
}

- (void) MarsstopNotifier
{
    if(reachabilityRef!= NULL)
    {
        //use CFRunLoopGetMain instead of CFRunLoopGetCurrent, because threads created by pthread do not have runloop
        SCNetworkReachabilityUnscheduleFromRunLoop(reachabilityRef, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    }
}

- (void) dealloc
{
    [self MarsstopNotifier];
    if(reachabilityRef!= NULL)
    {
        CFRelease(reachabilityRef);
    }
    [super dealloc];
}

+ (MarsReachability*) reachabilityWithHostName: (NSString*) hostName;
{
    MarsReachability* retVal = NULL;
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(NULL, [hostName UTF8String]);
    if(reachability!= NULL)
    {
        retVal= [[[self alloc] init] autorelease];
        if(retVal!= NULL)
        {
            retVal->reachabilityRef = reachability;
            retVal->localWiFiRef = NO;
        }
    }
    return retVal;
}

+ (MarsReachability*) reachabilityWithAddress: (const struct sockaddr*) hostAddress;
{
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)hostAddress);
    MarsReachability* retVal = NULL;
    if(reachability!= NULL)
    {
        retVal= [[[self alloc] init] autorelease];
        if(retVal!= NULL)
        {
            retVal->reachabilityRef = reachability;
            retVal->localWiFiRef = NO;
        }
    }
    return retVal;
}

+ (MarsReachability*) reachabilityForInternetConnection;
{
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;
    
    MarsReachability* netReach  =  [self reachabilityWithAddress: (const struct sockaddr*)&zeroAddress];
    if (NotReachable != [netReach currentReachabilityStatus]) return netReach;
    
    struct sockaddr_in6 zeroAddress6;
    bzero(&zeroAddress6, sizeof(zeroAddress6));
    zeroAddress6.sin6_len = sizeof(zeroAddress6);
    zeroAddress6.sin6_family = AF_INET6;
    
    return [self reachabilityWithAddress: (const struct sockaddr*)&zeroAddress6];
}

+ (MarsReachability*) reachabilityForLocalWiFi;
{
    struct sockaddr_in localWifiAddress;
    bzero(&localWifiAddress, sizeof(localWifiAddress));
    localWifiAddress.sin_len = sizeof(localWifiAddress);
    localWifiAddress.sin_family = AF_INET;
    // IN_LINKLOCALNETNUM is defined in <netinet/in.h> as 169.254.0.0
    localWifiAddress.sin_addr.s_addr = htonl(IN_LINKLOCALNETNUM);
    MarsReachability* retVal = [self reachabilityWithAddress: (const struct sockaddr*)&localWifiAddress];
    if(retVal!= NULL)
    {
        retVal->localWiFiRef = YES;
    }
    return retVal;
}
        
+ (MarsNetworkStatus) getCacheReachabilityStatus:(BOOL) flash
{
    if(!flash) return gs_Marsstatus;
    gs_Marsstatus = [[self reachabilityForInternetConnection] currentReachabilityStatus];
    return gs_Marsstatus;
}

#pragma mark Network Flag Handling

- (MarsNetworkStatus) localWiFiStatusForFlags: (SCNetworkReachabilityFlags) flags
{
    PrintReachabilityFlags(flags, "localWiFiStatusForFlags");

    MarsNetworkStatus retVal = NotReachable;
    if((flags & kSCNetworkReachabilityFlagsReachable) && (flags & kSCNetworkReachabilityFlagsIsDirect))
    {
        retVal = ReachableViaWiFi;    
    }
    return retVal;
}

- (MarsNetworkStatus) networkStatusForFlags: (SCNetworkReachabilityFlags) flags
{
    PrintReachabilityFlags(flags, "networkStatusForFlags");
    if ((flags & kSCNetworkReachabilityFlagsReachable) == 0)
    {
        // if target host is not reachable
        return NotReachable;
    }

    MarsNetworkStatus retVal = NotReachable;
    
    if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0)
    {
        // if target host is reachable and no connection is required
        //  then we'll assume (for now) that your on Wi-Fi
        retVal = ReachableViaWiFi;
    }
    
    
    if ((((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0))
    {
            // ... and the connection is on-demand (or on-traffic) if the
            //     calling application is using the CFSocketStream or higher APIs

            if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0)
            {
                // ... and no [user] intervention is needed
                retVal = ReachableViaWiFi;
            }
        }
#if TARGET_OS_IPHONE
    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN)
    {
        // ... but WWAN connections are OK if the calling application
        //     is using the CFNetwork (CFSocketStream?) APIs.
        retVal = ReachableViaWWAN;
    }
#endif
    return retVal;
}

- (BOOL) connectionRequired;
{
    NSAssert(reachabilityRef != NULL, @"connectionRequired called with NULL reachabilityRef");
    SCNetworkReachabilityFlags flags;
    if (SCNetworkReachabilityGetFlags(reachabilityRef, &flags))
    {
        return (flags & kSCNetworkReachabilityFlagsConnectionRequired);
    }
    return NO;
}

- (MarsNetworkStatus) currentReachabilityStatus
{
    NSAssert(reachabilityRef != NULL, @"currentNetworkStatus called with NULL reachabilityRef");
    MarsNetworkStatus retVal = NotReachable;
    SCNetworkReachabilityFlags flags;
    if (SCNetworkReachabilityGetFlags(reachabilityRef, &flags))
    {
        if(localWiFiRef)
        {
            retVal = [self localWiFiStatusForFlags: flags];
        }
        else
        {
            retVal = [self networkStatusForFlags: flags];
        }
    }
    return retVal;
}
@end

#endif
          
void comm_export_symbols_2(){}
          
    
          
