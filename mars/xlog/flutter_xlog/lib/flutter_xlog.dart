
import 'flutter_xlog_platform_interface.dart';

class FlutterXlog {
  Future<String?> getPlatformVersion() {
    return FlutterXlogPlatform.instance.getPlatformVersion();
  }
}
