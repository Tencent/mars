
//please include at the end of header file hook cpp file

#include "comm/debugger/spy_base.h"

#ifndef SPY_DISABLE

#include "comm/thread/tss.h"

#undef  XLOGGER_HOOK
#define XLOGGER_HOOK SPY_DEF_XLOGGER_HOOK

#define SPY_ATTACH_CLASS(name, ...) SpyCore::Singleton::Instance()->AddSpy(reinterpret_cast<void*>(this), name, new SPY_DEF_CLASS_NAME(reinterpret_cast<void*>(this), ##__VA_ARGS__))
#define SPY_DETACH_CLASS() SpyCore::Singleton::Instance()->RemoveSpy(reinterpret_cast<void*>(this))

#define SPY_ATTACH_VARIABLE(key, variable) SpyCore::Singleton::Instance()->GetSpy(reinterpret_cast<const void*>(this))->Attach(key, variable)
#define SPY_DETACH_VARIABLE(key) SpyCore::Singleton::Instance()->GetSpy(reinterpret_cast<const void*>(this))->Detach(key);

#define SPY_RANDOM_NAME(file, line) file #line

#define SPY_HOOK_THIS_API(func, ...) \
{\
static Tss s_func(NULL);\
\
if (NULL==s_func.get())\
{\
    s_func.set((void*) SPY_RANDOM_NAME(__FILE__, __LINE__));\
    return ((SPY_DEF_CLASS_NAME*)SpyCore::Singleton::Instance()->GetSpy(reinterpret_cast<const void*>(this)))->func(__VA_ARGS__);\
} else {\
    s_func.set(NULL);\
}\
}\

#else

#define SPY_ATTACH_CLASS(name, ...) ((void)0)
#define SPY_DETACH_CLASS() ((void)0)

#define SPY_ATTACH_VARIABLE(key, variable) ((void)0)
#define SPY_DETACH_VARIABLE(key) ((void)0)

#define SPY_HOOK_THIS_API(func, ...) ((void)0)
#endif
