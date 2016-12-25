/*
 * baseevent.h
 *
 *  Created on: 2016年3月24日
 *      Author: caoshaokun
 */

#ifndef BASEEVENT_INTERFACE_BASEEVENT_H_
#define BASEEVENT_INTERFACE_BASEEVENT_H_

#include <vector>
#include <string>

#ifdef ANDROID
#include <jni.h>

namespace mars {
namespace baseevent {

	extern void addLoadModule(std::string _module_name);

	extern jobject getLoadLibraries(JNIEnv *_env);

}
}

#endif

#endif /* BASEEVENT_INTERFACE_BASEEVENT_H_ */
