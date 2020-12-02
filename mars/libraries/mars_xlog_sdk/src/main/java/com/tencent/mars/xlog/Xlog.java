package com.tencent.mars.xlog;

public class Xlog implements Log.LogImp {

	public static final int LEVEL_ALL = 0;
	public static final int LEVEL_VERBOSE = 0;
	public static final int LEVEL_DEBUG = 1;
	public static final int LEVEL_INFO = 2;
	public static final int LEVEL_WARNING = 3;
	public static final int LEVEL_ERROR = 4;
	public static final int LEVEL_FATAL = 5;
	public static final int LEVEL_NONE = 6;

	public static final int COMPRESS_LEVEL1 = 1;
	public static final int COMPRESS_LEVEL2 = 2;
	public static final int COMPRESS_LEVEL3 = 3;
	public static final int COMPRESS_LEVEL4 = 4;
	public static final int COMPRESS_LEVEL5 = 5;
	public static final int COMPRESS_LEVEL6 = 6;
	public static final int COMPRESS_LEVEL7 = 7;
	public static final int COMPRESS_LEVEL8 = 8;
	public static final int COMPRESS_LEVEL9 = 9;

	public static final int AppednerModeAsync = 0;
	public static final int AppednerModeSync = 1;

	public static final int ZLIB_MODE = 0;
	public static final int ZSTD_MODE = 1;

	static class XLoggerInfo {
		public int level;
		public String tag;
		public String filename;
		public String funcname;
		public int line;
		public long pid;
		public long tid;
		public long maintid;
	}

	public static class XLogConfig {
		public int level = LEVEL_INFO;
		public int mode = AppednerModeAsync;
		public String logdir;
		public String nameprefix;
		public String pubkey = "";
		public int compressmode = ZLIB_MODE;
		public int compresslevel = 0;
		public String cachedir;
		public int cachedays = 0;
	}

    public static void open(boolean isLoadLib, int level, int mode, String cacheDir, String logDir, String nameprefix, String pubkey) {
		if (isLoadLib) {
			System.loadLibrary("c++_shared");
			System.loadLibrary("marsxlog");
		}

		XLogConfig logConfig = new XLogConfig();
		logConfig.level = level;
		logConfig.mode = mode;
		logConfig.logdir = logDir;
		logConfig.nameprefix = nameprefix;
		logConfig.pubkey = pubkey;
		logConfig.compressmode = ZLIB_MODE;
		logConfig.compresslevel = 0;
		logConfig.cachedir = cacheDir;
		logConfig.cachedays = 0;
		appenderOpen(logConfig);
	}

	private static String decryptTag(String tag) {
		return tag;
	}

	@Override
	public void logV(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_VERBOSE, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logD(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_DEBUG, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logI(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_INFO, decryptTag(tag), filename, funcname, line, pid, tid, maintid,  log);
	}

	@Override
	public void logW(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_WARNING, decryptTag(tag), filename, funcname, line, pid, tid, maintid,  log);
	}

	@Override
	public void logE(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_ERROR, decryptTag(tag), filename, funcname, line, pid, tid, maintid,  log);
	}

	@Override
	public void logF(long logInstancePtr, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(logInstancePtr, LEVEL_FATAL, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}


	@Override
	public void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {

		XLogConfig logConfig = new XLogConfig();
		logConfig.level = level;
		logConfig.mode = mode;
		logConfig.logdir = logDir;
		logConfig.nameprefix = nameprefix;
		logConfig.compressmode = ZLIB_MODE;
		logConfig.pubkey = "";
		logConfig.cachedir = cacheDir;
		logConfig.cachedays = cacheDays;

		appenderOpen(logConfig);
	}

	public static native void logWrite(XLoggerInfo logInfo, String log);

	public static void logWrite2(int level, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log){
		logWrite2(0, level, tag, filename ,funcname, line, pid, tid, maintid, log);
	}

	public static native void logWrite2(long logInstancePtr, int level, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log);

	@Override
	public native int getLogLevel(long logInstancePtr);


	@Override
	public native void setAppenderMode(long logInstancePtr, int mode);

	@Override
	public long openLogInstance(int level, int mode, String cacheDir, String logDir, String nameprefix, int cacheDays) {
		XLogConfig logConfig = new XLogConfig();
		logConfig.level = level;
		logConfig.mode = mode;
		logConfig.logdir = logDir;
		logConfig.nameprefix = nameprefix;
		logConfig.compressmode = ZLIB_MODE;
		logConfig.pubkey = "";
		logConfig.cachedir = cacheDir;
		logConfig.cachedays = cacheDays;
		return newXlogInstance(logConfig);
	}

	@Override
	public native long getXlogInstance(String nameprefix);

	@Override
	public native void releaseXlogInstance(String nameprefix);

	public native long newXlogInstance(XLogConfig logConfig);

	@Override
	public native void setConsoleLogOpen(long logInstancePtr, boolean isOpen);	//set whether the console prints log

	private static native void appenderOpen(XLogConfig logConfig);

	@Override
	public native void appenderClose();

	@Override
	public native void appenderFlush(long logInstancePtr, boolean isSync);

	@Override
	public native void setMaxFileSize(long logInstancePtr, long aliveSeconds);

	@Override
	public native void setMaxAliveTime(long logInstancePtr, long aliveSeconds);

}