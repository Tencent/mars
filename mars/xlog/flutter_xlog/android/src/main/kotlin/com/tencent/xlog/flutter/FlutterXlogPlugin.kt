package com.tencent.xlog.flutter

import com.tencent.mars.xlog.Log
import com.tencent.mars.xlog.Xlog
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result

/** FlutterXLogPlugin */
class FlutterXLogPlugin: FlutterPlugin, MethodCallHandler {
  private lateinit var channel : MethodChannel

  init {
    System.loadLibrary("c++_shared")
    System.loadLibrary("marsxlog")
  }

  private val xlog = Xlog()

  override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "flutter_xlog")
    channel.setMethodCallHandler(this)
  }

  override fun onMethodCall(call: MethodCall, result: Result) {
    if (call.method == "open") {
      val level = call.argument<Int>("level") ?: Log.LEVEL_INFO
      val cacheDir = call.argument<String>("cacheDir")
      val logDir = call.argument<String>("logDir")
      val namePrefix = call.argument<String>("namePrefix")
      val cacheDays = call.argument<Int>("cacheDays") ?: 0
      val consoleLogOpen = call.argument<Boolean>("consoleLogOpen") ?: false
      Log.setLogImp(xlog)
      Log.setLevel(level, false)
      Log.setConsoleLogOpen(consoleLogOpen)
      xlog.appenderOpen(level, Xlog.AppednerModeAsync, cacheDir, logDir, namePrefix, cacheDays)
      result.success(null)
    } else if (call.method == "close") {
      xlog.appenderClose()
    } else {
      result.notImplemented()
    }
  }

  override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }
}
