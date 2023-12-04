
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'package:ffi/ffi.dart';
import 'flutter_xlog_plugin.dart';

enum LogLevel {
  VERBOSE,
  DEBUG,
  INFO,
  WARNNING,
  ERROR,
  FATAL,
  NONE
}

class XLogConfig {
  final LogLevel level;
  final String cacheDir;
  final String logDir;
  final String namePrefix;
  final int cacheDays;
  final bool consoleLogOpen;
  final String pubKey;

  XLogConfig({
    this.level = LogLevel.INFO,
    required this.cacheDir,
    required this.logDir,
    this.namePrefix = 'mars_xlog',
    this.cacheDays = 0,
    this.consoleLogOpen = false,
    this.pubKey = ''
  });
}

class XLog {

  static final FlutterXLogPlugin _xLogPlugin = FlutterXLogPlugin();

  static final DynamicLibrary _dylib = Platform.isAndroid
      ? DynamicLibrary.open('libmarsxlog.so')
      : DynamicLibrary.process();

  static final _fun_write_log = _dylib.lookupFunction<Void Function(UintPtr, Int32, Pointer<Utf8>, Int32, Int32, Int32, Pointer<Utf8>),
      void Function(int instancePtr, int level, Pointer<Utf8> tag, int pid, int tid, int mainTid, Pointer<Utf8> log)>("Dart_XloggerWrite");

  static Future<void> open(XLogConfig config) async {
      _xLogPlugin.open(config.level.index, config.cacheDir, config.logDir, config.namePrefix, config.cacheDays, config.consoleLogOpen, config.pubKey);
  }

  static void v(String tag, String msg) {
    _writeLog(LogLevel.VERBOSE, tag, msg);
  }

  static void d(String tag, String msg) {
    _writeLog(LogLevel.DEBUG, tag, msg);
  }

  static void i(String tag, String msg) {
    _writeLog(LogLevel.INFO, tag, msg);
  }

  static void w(String tag, String msg) {
    _writeLog(LogLevel.WARNNING, tag, msg);
  }

  static void e(String tag, String msg) {
    _writeLog(LogLevel.ERROR, tag, msg);
  }

  static void close() async {
    _xLogPlugin.close();
  }

  static void _writeLog(LogLevel level, String tag, String msg) {
    final tagPtr = tag.toNativeUtf8();
    final msgPtr = '[${Isolate.current.debugName}] $msg'.toNativeUtf8();
    _fun_write_log(0, level.index, tagPtr, pid, 0, 0, msgPtr);
    malloc.free(tagPtr);
    malloc.free(msgPtr);
  }
}
