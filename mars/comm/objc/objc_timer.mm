//
//  objc_timer.cpp
//  MicroMessenger
//
//  Created by yerungui on 12-12-10.
//

#include "comm/objc/objc_timer.h"
#import <Foundation/Foundation.h>
#include <list>

#include "comm/thread/lock.h"
#include "comm/alarm.h"

void onAlarm(long long _id)
{
//    Alarm::OnAlarm(reinterpret_cast<Alarm*>(_id));
}

@interface TimerRuner : NSObject
-(void) Run:(NSTimer*)timer;
@end

struct TimerRecord
{
    TimerRecord()
    {
        id_ = 0;
        timer = nil;
        timer_runer = nil;
    }
    
    long long id_;
    NSTimer* timer;
    TimerRuner* timer_runer;
};

static std::list<TimerRecord> gs_lst_timer_record;
static Mutex gs_mutex;

@implementation TimerRuner

-(void) Run:(NSTimer*)timer
{
    ScopedLock lock(gs_mutex);
    for (std::list<TimerRecord>::iterator it = gs_lst_timer_record.begin(); it!=gs_lst_timer_record.end(); ++it)
    {
        if (timer==it->timer)
        {
            if (nil!=it->timer)
            {
                [it->timer invalidate];
                [it->timer release];
                it->timer = nil;
            }
            if (nil!=it->timer_runer)
            {
                [it->timer_runer release];
                it->timer_runer = nil;
            }
            
            long long id_ = it->id_;
            gs_lst_timer_record.erase(it);

            onAlarm(id_);
            break;
        }
    }
}

@end

bool StartAlarm(long long _id, int after)
{
    ScopedLock lock(gs_mutex);
    for (std::list<TimerRecord>::iterator it = gs_lst_timer_record.begin(); it!=gs_lst_timer_record.end(); ++it)
    {
        if (_id==it->id_) return false;
    }
    
    NSTimeInterval  interval = (NSTimeInterval)after/1000;
    NSRunLoop *runLoop = [NSRunLoop mainRunLoop];
    TimerRecord tr;
    tr.id_ = _id;
    tr.timer_runer = [[TimerRuner alloc] init];
    tr.timer = [NSTimer timerWithTimeInterval:interval target:tr.timer_runer selector:@selector(Run:) userInfo:tr.timer repeats:NO];
    [tr.timer retain];
    [runLoop addTimer:tr.timer forMode:NSDefaultRunLoopMode];
    gs_lst_timer_record.push_back(tr);
    return true;
}

bool StopAlarm(long long _id)
{
    ScopedLock lock(gs_mutex);
    for (std::list<TimerRecord>::iterator it = gs_lst_timer_record.begin(); it!=gs_lst_timer_record.end(); ++it)
    {
        if (_id==it->id_)
        {
            if (nil!=it->timer)
            {
                [it->timer invalidate];
                [it->timer release];
                it->timer = nil;
            }
            if (nil!=it->timer_runer)
            {
                [it->timer_runer release];
                it->timer_runer = nil;
            }
            gs_lst_timer_record.erase(it);
            return true;
        }
    }
    return false;
}

void comm_export_symbols_0(){}
