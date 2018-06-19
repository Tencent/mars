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

	public static final int AppednerModeAsync = 0;
	public static final int AppednerModeSync = 1;

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

	public static void open(boolean isLoadLib, int level, int mode, String cacheDir, String logDir, String nameprefix, String pubkey) {
		if (isLoadLib) {
			System.loadLibrary("stlport_shared");
			System.loadLibrary("marsxlog");
		}

		appenderOpen(level, mode, cacheDir, logDir, nameprefix, pubkey);
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

	public static native void appenderOpen(int level, int mode, String cacheDir, String logDir, String nameprefix, String pubkey);

	@Override
	public native void appenderClose();

	@Override
	public native void appenderFlush(boolean isSync);

}
