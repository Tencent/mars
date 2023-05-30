package com.tencent.mars.xlog;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Process;
import android.widget.Toast;

import java.util.HashMap;
import java.util.Map;


/**
 * @author zhaoyuan zhangweizang
 */
public class Log {
    private static final String TAG = "mars.xlog.log";

    public static final int LEVEL_VERBOSE = 0;
    public static final int LEVEL_DEBUG = 1;
    public static final int LEVEL_INFO = 2;
    public static final int LEVEL_WARNING = 3;
    public static final int LEVEL_ERROR = 4;
    public static final int LEVEL_FATAL = 5;
    public static final int LEVEL_NONE = 6;

    // defaults to LEVEL_NONE
    private static int level = LEVEL_NONE;
    public static Context toastSupportContext = null;

    public interface LogImp {

        void logV(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logI(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logD(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logW(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logE(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        void logF(long logInstancePtr, String tag, String filename, String funcname, int linuxTid, int pid, long tid, long maintid, String log);

        int getLogLevel(long logInstancePtr);

        void setAppenderMode(long logInstancePtr, int mode);

        long openLogInstance(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays);

        long getXlogInstance(String nameprefix);

        void releaseXlogInstance(String nameprefix);

        void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays);

        void appenderClose();

        void appenderFlush(long logInstancePtr, boolean isSync);

        void setConsoleLogOpen(long logInstancePtr, boolean isOpen);

        void setMaxFileSize(long logInstancePtr, long aliveSeconds);

        void setMaxAliveTime(long logInstancePtr, long aliveSeconds);

    }

    private static LogImp debugLog = new LogImp() {
        private Handler handler = new Handler(Looper.getMainLooper());

        @Override
        public void logV(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_VERBOSE) {
                android.util.Log.v(tag, log);
            }
        }

        @Override
        public void logI(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_INFO) {
                android.util.Log.i(tag, log);
            }
        }

        @Override
        public void logD(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_DEBUG) {
                android.util.Log.d(tag, log);
            }

        }

        @Override
        public void logW(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_WARNING) {
                android.util.Log.w(tag, log);
            }

        }

        @Override
        public void logE(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
            if (level <= LEVEL_ERROR) {
                android.util.Log.e(tag, log);
            }
        }


        @Override
        public void logF(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, final String log) {
            if (level > LEVEL_FATAL) {
                return;
            }
            android.util.Log.e(tag, log);
            if (toastSupportContext != null) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(toastSupportContext, log, Toast.LENGTH_LONG).show();
                    }
                });
            }
        }

        @Override
        public int getLogLevel(long logInstancePtr) {
            return level;
        }

        @Override
        public void setAppenderMode(long logInstancePtr, int mode) {

        }

        @Override
        public long openLogInstance(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {
            return 0;
        }

        @Override
        public long getXlogInstance(String nameprefix) {
            return 0;
        }

        @Override
        public void releaseXlogInstance(String nameprefix) {

        }

        @Override
        public void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {

        }

        @Override
        public void appenderClose() {

        }

        @Override
        public void appenderFlush(long logInstancePtr, boolean isSync) {
        }

        @Override
        public void setConsoleLogOpen(long logInstancePtr, boolean isOpen) {

        }

        @Override
        public void setMaxAliveTime(long logInstancePtr, long aliveSeconds) {

        }

        @Override
        public void setMaxFileSize(long logInstancePtr, long aliveSeconds) {

        }

    };

    private static LogImp logImp = debugLog;

    public static void setLogImp(LogImp imp) {
        logImp = imp;
    }

    public static LogImp getImpl() {
        return logImp;
    }

    public static void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {
        if (logImp != null) {
            logImp.appenderOpen(level, mode, cacheDir, logDir, nameprefix, cacheDays);
        }
    }

    public static void appenderClose() {
        if (logImp != null) {
            logImp.appenderClose();
            for (Map.Entry<String, LogInstance> entry : sLogInstanceMap.entrySet()) {
                closeLogInstance(entry.getKey());
            }
        }
    }

    public static void appenderFlush() {
        if (logImp != null) {
            logImp.appenderFlush(0, false);
            for (Map.Entry<String, LogInstance> entry : sLogInstanceMap.entrySet()) {
                entry.getValue().appenderFlush();
            }
        }
    }

    public static void appenderFlushSync(boolean isSync) {
        if (logImp != null) {
            logImp.appenderFlush(0, isSync);

        }
    }

    public static int getLogLevel() {
        if (logImp != null) {
            return logImp.getLogLevel(0);
        }
        return LEVEL_NONE;
    }

    public static void setLevel(final int level, final boolean jni) {
        Log.level = level;
        android.util.Log.w(TAG, "new log level: " + level);

        if (jni) {
            android.util.Log.e(TAG, "no jni log level support");
        }
    }

    public static void setConsoleLogOpen(boolean isOpen) {
        if (logImp != null) {
            logImp.setConsoleLogOpen(0, isOpen);
        }
    }

    /**
     * use f(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void f(final String tag, final String msg) {
        f(tag, msg, (Object[]) null);
    }

    /**
     * use e(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void e(final String tag, final String msg) {
        e(tag, msg, (Object[]) null);
    }

    /**
     * use w(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void w(final String tag, final String msg) {
        w(tag, msg, (Object[]) null);
    }

    /**
     * use i(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void i(final String tag, final String msg) {
        i(tag, msg, (Object[]) null);
    }

    /**
     * use d(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void d(final String tag, final String msg) {
        d(tag, msg, (Object[]) null);
    }

    /**
     * use v(tag, format, obj) instead
     *
     * @param tag
     * @param msg
     */
    public static void v(final String tag, final String msg) {
        v(tag, msg, (Object[]) null);
    }

    public static void f(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_FATAL) {
            final String log = obj == null ? format : String.format(format, obj);
            logImp.logF(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void e(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logE(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void w(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_WARNING) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logW(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void i(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_INFO) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logI(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void d(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_DEBUG) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logD(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void v(String tag, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_VERBOSE) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            logImp.logV(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    public static void printErrStackTrace(String tag, Throwable tr, final String format, final Object... obj) {
        if (logImp != null && logImp.getLogLevel(0) <= LEVEL_ERROR) {
            String log = obj == null ? format : String.format(format, obj);
            if (log == null) {
                log = "";
            }
            log += "  " + android.util.Log.getStackTraceString(tr);
            logImp.logE(0, tag, "", "", 0, Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
        }
    }

    // private static final String SYS_INFO;

    // static {
    //     final StringBuilder sb = new StringBuilder();
    //     try {
    //         sb.append("VERSION.RELEASE:[" + android.os.Build.VERSION.RELEASE);
    //         sb.append("] VERSION.CODENAME:[" + android.os.Build.VERSION.CODENAME);
    //         sb.append("] VERSION.INCREMENTAL:[" + android.os.Build.VERSION.INCREMENTAL);
    //         sb.append("] BOARD:[" + android.os.Build.BOARD);
    //         sb.append("] DEVICE:[" + android.os.Build.DEVICE);
    //         sb.append("] DISPLAY:[" + android.os.Build.DISPLAY);
    //         sb.append("] FINGERPRINT:[" + android.os.Build.FINGERPRINT);
    //         sb.append("] HOST:[" + android.os.Build.HOST);
    //         sb.append("] MANUFACTURER:[" + android.os.Build.MANUFACTURER);
    //         sb.append("] MODEL:[" + android.os.Build.MODEL);
    //         sb.append("] PRODUCT:[" + android.os.Build.PRODUCT);
    //         sb.append("] TAGS:[" + android.os.Build.TAGS);
    //         sb.append("] TYPE:[" + android.os.Build.TYPE);
    //         sb.append("] USER:[" + android.os.Build.USER + "]");
    //     } catch (Throwable e) {
    //         e.printStackTrace();
    //     }

    //     SYS_INFO = sb.toString();
    // }

    // public static String getSysInfo() {
    //     return SYS_INFO;
    // }

    private static Map<String, LogInstance> sLogInstanceMap = new HashMap<>();

    public static LogInstance openLogInstance(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {
        synchronized (sLogInstanceMap) {
            if (sLogInstanceMap.containsKey(nameprefix)) {
                return sLogInstanceMap.get(nameprefix);
            }
            LogInstance instance = new LogInstance(level, mode, cacheDir, logDir, nameprefix, cacheDays);
            sLogInstanceMap.put(nameprefix, instance);
            return instance;
        }
    }

    public static void closeLogInstance(String prefix) {
        synchronized (sLogInstanceMap) {
            if (null != logImp) {
                if (sLogInstanceMap.containsKey(prefix)) {
                    LogInstance logInstance = sLogInstanceMap.remove(prefix);
                    logImp.releaseXlogInstance(prefix);
                    logInstance.mLogInstancePtr = 0;
                }
            }
        }
    }

    public static LogInstance getLogInstance(String prefix) {
        synchronized (sLogInstanceMap) {
            if (sLogInstanceMap.containsKey(prefix)) {
                return sLogInstanceMap.get(prefix);
            }
            return null;
        }
    }

    public static class LogInstance {

        private long mLogInstancePtr = 0;

        private String mPrefix = null;

        private LogInstance(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {
            if (logImp != null) {
                mLogInstancePtr = logImp.openLogInstance(level, mode, cacheDir, logDir, nameprefix, cacheDays);
                mPrefix = nameprefix;
            }
        }

        public void f(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_FATAL && mLogInstancePtr != 0) {
                final String log = obj == null ? format : String.format(format, obj);
                logImp.logF(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }

        public void e(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_ERROR && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                logImp.logE(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }

        public void w(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_WARNING && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                logImp.logW(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }

        public void i(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_INFO && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                logImp.logI(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }

        public void d(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_DEBUG && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                logImp.logD(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }

        public void v(String tag, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_VERBOSE && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                logImp.logV(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }


        public void printErrStackTrace(String tag, Throwable tr, final String format, final Object... obj) {
            if (logImp != null && getLogLevel() <= LEVEL_ERROR && mLogInstancePtr != 0) {
                String log = obj == null ? format : String.format(format, obj);
                if (log == null) {
                    log = "";
                }
                log += "  " + android.util.Log.getStackTraceString(tr);
                logImp.logE(mLogInstancePtr, tag, "", "", Process.myTid(), Process.myPid(), Thread.currentThread().getId(), Looper.getMainLooper().getThread().getId(), log);
            }
        }


        public void appenderFlush() {
            if (logImp != null && mLogInstancePtr != 0) {
                logImp.appenderFlush(mLogInstancePtr, false);
            }
        }

        public void appenderFlushSync() {
            if (logImp != null && mLogInstancePtr != 0) {
                logImp.appenderFlush(mLogInstancePtr, true);
            }
        }

        public int getLogLevel() {
            if (logImp != null && mLogInstancePtr != 0) {
                return logImp.getLogLevel(mLogInstancePtr);
            }
            return LEVEL_NONE;
        }

        public void setConsoleLogOpen(boolean isOpen) {
            if (null != logImp && mLogInstancePtr != 0) {
                logImp.setConsoleLogOpen(mLogInstancePtr, isOpen);
            }
        }


    }
}
