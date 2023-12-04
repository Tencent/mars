import Flutter
import UIKit

public class FlutterXLogPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "flutter_xlog", binaryMessenger: registrar.messenger())
    let instance = FlutterXLogPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "open":
        var bridge = XLogBridge();
        let params = call.arguments as! [String:Any];
        let level = params["level"] as! UInt;
        let cacheDir = params["cacheDir"] as! String;
        let logDir = params["logDir"] as! String;
        let namePrefix = params["namePrefix"] as! String;
        let cacheDays = params["cacheDays"] as! UInt;
        let consoleLogOpen = params["consoleLogOpen"] as! Bool;
        let pubKey = params["pubKey"] as! String;
        bridge.open(level, cacheDir: cacheDir, logDir: logDir, prefix:namePrefix, cacheDays: cacheDays, consoleLogOpen: consoleLogOpen);
        result(nil);
    case "close":
        result(nil);
    default:
        result(FlutterMethodNotImplemented)
    }
  }
}
