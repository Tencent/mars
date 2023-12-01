import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_xlog/flutter_xlog.dart';
import 'package:flutter_xlog/flutter_xlog_platform_interface.dart';
import 'package:flutter_xlog/flutter_xlog_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFlutterXlogPlatform
    with MockPlatformInterfaceMixin
    implements FlutterXlogPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final FlutterXlogPlatform initialPlatform = FlutterXlogPlatform.instance;

  test('$MethodChannelFlutterXlog is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlutterXlog>());
  });

  test('getPlatformVersion', () async {
    FlutterXlog flutterXlogPlugin = FlutterXlog();
    MockFlutterXlogPlatform fakePlatform = MockFlutterXlogPlatform();
    FlutterXlogPlatform.instance = fakePlatform;

    expect(await flutterXlogPlugin.getPlatformVersion(), '42');
  });
}
