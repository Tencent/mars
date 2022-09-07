 //
 // Created by Cpan on 2022/9/7.
 //


 #include <jni.h>

 #include "comm/jni/jnicat/jnicat_core.h"
 #include "comm/jni/jnicat/jnicat_object_wrapper.h"

 #include "mars/app/app.h"
 #include "mars/app/app_manager.h"
 #include "mars/app/jni/app_manager_callback_wrapper.h"


 namespace mars {
 namespace app {

 class JniAppManager {
  public:
     static void JniCreateAppManagerFromHandle(JNIEnv *env, jobject instance, jlong handle) {
         auto app_manager_cpp = (AppManager *) j2c_cast(handle);
         auto appManangerWrapper = new jnicat::JniObjectWrapper<AppManager>(app_manager_cpp);
         appManangerWrapper->instantiate(env, instance);
     }

     static void JniOnDestroyApplication(JNIEnv *env, jobject instance) {
         auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
         jnicat::JniObjectWrapper<Callback>::dispose(env, instance,
                                                                      "callbackHandle");
         app_manager_cpp->SetCallback(nullptr);
         jnicat::JniObjectWrapper<AppManager>::dispose(env, instance);
     }

     static void JniSetCallback(JNIEnv *env, jobject instance, jobject callback) {
         auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
         auto appManagerJniCallback = new AppManagerJniCallback(env, callback);
         auto appManagerCallbackWrapper =
                 new jnicat::JniObjectWrapper<AppManagerJniCallback>(appManagerJniCallback);
         appManagerCallbackWrapper->instantiate(env, instance, "callbackHandle");
         app_manager_cpp->SetCallback(appManagerJniCallback);
     }
 };
 }  // namespace app
 }  // namespace mars