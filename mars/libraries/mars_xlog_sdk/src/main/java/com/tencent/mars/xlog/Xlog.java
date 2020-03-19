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

    static class XLogConfig {
        public int level;
        public int mode;
        public String logdir;
        public String nameprefix;
        public String pubkey;
        public int compressmode;
        public int compresslevel;
        public String cachedir;
        public int cachedays;
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
		logConfig.compressmode = ZSTD_MODE;
		logConfig.compresslevel = COMPRESS_LEVEL6;
		logConfig.withcache = false;
		logConfig.cachedir = cacheDir;
		logConfig.cachedays = 0;
		appenderOpen(logConfig);
	}

	private static String decryptTag(String tag) {
		return tag;
	}

	@Override
	public void logV(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_VERBOSE, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logD(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_DEBUG, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logI(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_INFO, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logW(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_WARNING, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logE(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_ERROR, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}

	@Override
	public void logF(String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log) {
		logWrite2(LEVEL_FATAL, decryptTag(tag), filename, funcname, line, pid, tid, maintid, log);
	}


	public static native void logWrite(XLoggerInfo logInfo, String log);

	public static native void logWrite2(int level, String tag, String filename, String funcname, int line, int pid, long tid, long maintid, String log);

	@Override
	public native int getLogLevel();

	public static native void setLogLevel(int logLevel);

	public static native void setAppenderMode(int mode);

	public static native void setConsoleLogOpen(boolean isOpen);	//set whether the console prints log

	public static native void setErrLogOpen(boolean isOpen);	//set whether the  prints err log into a separate file

	public static native void appenderOpen(XLogConfig logConfig);

	public static native void setMaxFileSize(long size);

	/**
	 * should be called before appenderOpen to take effect
	 * @param duration alive seconds
	 */
	public static native void setMaxAliveTime(long duration);

	@Override
	public native void appenderClose();

	@Override
	public native void appenderFlush(boolean isSync);

}
