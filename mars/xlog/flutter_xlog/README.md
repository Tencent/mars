# flutter_xlog

A plugin for use [Mars XLog](https://github.com/Tencent/mars) in flutter project.

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


