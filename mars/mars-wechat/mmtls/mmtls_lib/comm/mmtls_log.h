#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <sys/cdefs.h>

#ifndef WIN32
#include <libgen.h>
#endif

namespace mmtls {
struct LogImpt_t;
class ILog {
 public:
    /**
            @brief enum���ͣ���־������
    */
    enum {
        MMTLS_LOG_DISABLE = 0, /**< ��ֹ��־���  */
        MMTLS_LOG_INFO = 1,    /**< info��־������־�������infoĿ¼��  */
        MMTLS_LOG_IMPT = 1,    /**< important��־����������¼��Ҫ������־ */
        MMTLS_LOG_INIT = 2, /**< init��־����һ������ģ���ʼ��ʱ����־���  */
        MMTLS_LOG_ERR =
            2,             /**<
                            * error��־�������г��ִ���ʱ���������ϵͳ������߼�����
                            */
        MMTLS_LOG_MSG = 2, /**< msg��־����������¼�Ǵ������������Ҫ��¼����״̬�Ĳ��� */
        MMTLS_LOG_DEBUG = 3,   /**< debug��־���� */
        MMTLS_LOG_MONITOR = 8, /**< monitor��־���� */
        MMTLS_LOG_IN = 16,     /**< �����������־���� */
        MMTLS_LOG_ALL = 32     /**< ���������־ */
    };
    static ILog* GetDefault();
    ILog();
    ~ILog();

    bool IsDiscard(int level) const;

    /*@note init log params
     *@params module_name logname
     *@params loglevel
     *@params max_size
     */
    int Init(const char* module_name, const int logLevel, const char* path, const int max_size);

    void Destory();
    void Log(int level, const char* func, const char* file_basename, unsigned int line, const char* format, ...)
        __attribute__((format(printf, 6, 7)));
    void LightLog(int level, const char* format, ...) __attribute__((format(printf, 3, 4)));

 protected:
    LogImpt_t* impt_;
};
extern ILog g_Log;

#define __MMTLS_LOG(handler, loglevel, format, ...)                                     \
    do {                                                                                \
        if (!(handler).IsDiscard(loglevel)) {                                           \
            handler.Log(loglevel, __func__, __FILE__, __LINE__, format, ##__VA_ARGS__); \
        }                                                                               \
    } while (0)

#define __MMTLS_LIGHT_LOG(handler, loglevel, format, ...)      \
    do {                                                       \
        if (!(handler).IsDiscard(loglevel)) {                  \
            handler.LightLog(loglevel, format, ##__VA_ARGS__); \
        }                                                      \
    } while (0)

#define MMTLS_INITLOG(module_name, logLevel, path, max_size) mmtls::g_Log.Init(module_name, logLevel, path, max_size)

#define MMTLSLOG_ERR(format, ...) __MMTLS_LOG(mmtls::g_Log, ILog::MMTLS_LOG_ERR, format, ##__VA_ARGS__);
#define MMTLSLOG_DEBUG(format, ...) __MMTLS_LOG(mmtls::g_Log, ILog::MMTLS_LOG_DEBUG, format, ##__VA_ARGS__);
#define MMTLSLOG_INIT(format, ...) __MMTLS_LOG(mmtls::g_Log, ILog::MMTLS_LOG_INIT, format, ##__VA_ARGS__);
#define MMTLSLOG_MSG(format, ...) __MMTLS_LOG(mmtls::g_Log, ILog::MMTLS_LOG_MSG, format, ##__VA_ARGS__);
#define MMTLSLOG_IMPT(format, ...) __MMTLS_LOG(mmtls::g_Log, ILog::MMTLS_LOG_IMPT, format, ##__VA_ARGS__);

#define MMTLSLOG_LIGHT_ERR(format, ...) __MMTLS_LIGHT_LOG(mmtls::g_Log, ILog::MMTLS_LOG_ERR, format, ##__VA_ARGS__);
#define MMTLSLOG_LIGHT_DEBUG(format, ...) __MMTLS_LIGHT_LOG(mmtls::g_Log, ILog::MMTLS_LOG_DEBUG, format, ##__VA_ARGS__);
#define MMTLSLOG_LIGHT_INIT(format, ...) __MMTLS_LIGHT_LOG(mmtls::g_Log, ILog::MMTLS_LOG_INIT, format, ##__VA_ARGS__);
#define MMTLSLOG_LIGHT_MSG(format, ...) __MMTLS_LIGHT_LOG(mmtls::g_Log, ILog::MMTLS_LOG_MSG, format, ##__VA_ARGS__);
#define MMTLSLOG_LIGHT_IMPT(format, ...) __MMTLS_LIGHT_LOG(mmtls::g_Log, ILog::MMTLS_LOG_IMPT, format, ##__VA_ARGS__);

};  // namespace mmtls
