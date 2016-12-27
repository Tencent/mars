//please include at the start pos of spy impl cpp file

#ifdef SPY_DEF_CLASS_NAME
#undef SPY_DEF_CLASS_NAME
#else
#error "SPY_DEF_CLASS_NAME not defined"
#endif

#ifdef SPY_DEF_XLOGGER_HOOK
#undef SPY_DEF_XLOGGER_HOOK
#else
#error "SPY_DEF_XLOGGER_HOOK not defined"
#endif

#define private   public
#define protected public
