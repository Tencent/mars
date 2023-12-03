import 'package:flutter/services.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class FlutterXLogPlugin extends PlatformInterface {

  FlutterXLogPlugin() : super(token: _token);

  static final Object _token = Object();

  final methodChannel = const MethodChannel('flutter_xlog');

  Future<void> open(int level, String cacheDir, String logDir, String namePrefix, int cacheDays, bool consoleLogOpen) {
    return methodChannel.invokeMethod<void>('open', {
      'level': level,
      'cacheDir': cacheDir,
      'logDir': logDir,
      'namePrefix': namePrefix,
      'cacheDays': cacheDays,
      'consoleLogOpen': consoleLogOpen
    });
  }

  Future<void> close() {
    return methodChannel.invokeMethod<void>('close');
  }


}
