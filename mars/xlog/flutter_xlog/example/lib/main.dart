import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_xlog/flutter_xlog.dart';
import 'package:path_provider/path_provider.dart';

var cacheDir = '';
var logDir = '';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  cacheDir = (await getApplicationCacheDirectory()).path;
  logDir = '';
  if (Platform.isAndroid) {
    logDir = (await getExternalCacheDirectories())?[0].path ?? cacheDir;
  } else if (Platform.isIOS) {
    logDir = (await getApplicationSupportDirectory()).path;
  }
  await XLog.open(XLogConfig(cacheDir: cacheDir, logDir: logDir, consoleLogOpen: true));
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {

  @override
  Widget build(BuildContext context) {
    XLog.i("MyApp", "build _MyAppState");
    compute((message) {
      XLog.i("MyApp", "$message in Isolate");
    }, "compute in build");
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('FlutterXLog example app'),
        ),
        body: Column (
            children: [
              Text('Cached XLog in $cacheDir \nWrite XLog to $logDir \n'),
              TextButton(onPressed: () {
                XLog.i("MyApp", "click");
                XLog.close();
              }, child: const Text('close XLog'))
            ]
        ),
      ),
    );
  }
}
