import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_xlog_platform_interface.dart';

/// An implementation of [FlutterXlogPlatform] that uses method channels.
class MethodChannelFlutterXlog extends FlutterXlogPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_xlog');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
