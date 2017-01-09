/*
 * OnAlarm.cpp
 *
 *  Created on: 2014-1-13
 *      Author: yerungui
 */


#include <jni.h>

extern "C" JNIEXPORT void JNICALL Java_com_tencent_mars_comm_Alarm_onAlarm(JNIEnv *, jclass, jlong id)
{
    xdebug2(TSF"BroadcastMessage seq:%_", (long long)id);
    MessageQueue::BroadcastMessage(MessageQueue::GetDefMessageQueue(), MessageQueue::Message(KALARM_MESSAGETITLE, (long long)id, 0));
}
