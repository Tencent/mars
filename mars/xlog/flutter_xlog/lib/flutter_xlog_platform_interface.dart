import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_xlog_method_channel.dart';

abstract class FlutterXlogPlatform extends PlatformInterface {
  /// Constructs a FlutterXlogPlatform.
  FlutterXlogPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterXlogPlatform _instance = MethodChannelFlutterXlog();

  /// The default instance of [FlutterXlogPlatform] to use.
  ///
  /// Defaults to [MethodChannelFlutterXlog].
  static FlutterXlogPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FlutterXlogPlatform] when
  /// they register themselves.
  static set instance(FlutterXlogPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
