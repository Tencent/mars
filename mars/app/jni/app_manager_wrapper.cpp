// //
// // Created by Cpan on 2022/9/7.
// //


// #include <jni.h>

// #include "comm/jni/jnicat/jnicat_core.h"
// #include "comm/jni/jnicat/jnicat_object_wrapper.h"

// #include "mars/app/app.h"
// #include "mars/app/app_manager.h"


// namespace mars {
// namespace app {

// class JniAppManager {
//  public:
//     static void JniCreateAppManagerFromHandle(JNIEnv *env, jobject instance, jlong handle) {
//         auto application_cpp = (AppManager *) j2c_cast(handle);
//         auto applicationWrapper = new jnicat::JniObjectWrapper<AppManager>(application_cpp);
//         applicationWrapper->instantiate(env, instance);
//     }

//     static void JniOnDestroyApplication(JNIEnv *env, jobject instance) {
//         auto application_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
//         jnicat::JniObjectWrapper<Callback>::dispose(env, instance,
//                                                                      "callbackHandle");
//         application_cpp->SetCallback(nullptr);
//         jnicat::JniObjectWrapper<AppManager>::dispose(env, instance);
//     }

//     static void JniSetCallback(JNIEnv *env, jobject instance, jobject callback) {
//         auto application_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
//         auto applicationJniCallback = new newt::ApplicationJniCallback(env, callback);
//         auto applicationCallbackWrapper =
//                 new jnicat::JniObjectWrapper<newt::ApplicationJniCallback>(applicationJniCallback);
//         applicationCallbackWrapper->instantiate(env, instance, "callbackHandle");
//         application_cpp->SetCallback(applicationJniCallback);
//     }
// };
// }  // namespace app
// }  // namespace mars