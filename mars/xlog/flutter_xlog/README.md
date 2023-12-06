# flutter_xlog

A plugin for use [Mars-XLog](https://github.com/Tencent/mars) in flutter project.

## Usage

### import package
```

import 'package:flutter_xlog/flutter_xlog.dart'

```
### open XLog
```

await XLog.open(XLogConfig(cacheDir: cacheDir, logDir: logDir, consoleLogOpen: true));

```
### write log
```

XLog.i("MyApp", "build _MyAppState");

```

### close XLog
You maybe want to close XLog because of switch user's account.
```

XLog.close()

```

## Android Issues

### keep XLog classes in proguard file
```
-keep class com.tencent.mars.** {*;}
```

### pick libc++_shared
When you run with debug mode, you may encounter a problem like this:
```
    2 files found with path 'lib/arm64-v8a/libc++_shared.so' from inputs:
    - xxxx
    - xxxx
```
To solve that problem, we recommend you to copy the libc++_shared.so to your app project and then pick it in build.gradle like this:
```
    packagingOptions {
        pickFirst 'jniLibs/armeabi-v7a/libc++_shared.so'
        pickFirst 'jniLibs/arm64-v8a/libc++_shared.so'
    }
```

### iOS Issues

Mars-XLog currently does not support iOS simulator. So you can only use it on iphone devices.

