//
//  AppDelegate.swift
//  XloggerSwiftDemo
//
//  Created by Jinkey on 2017/1/3.
//  Copyright © 2017年 Jinkey. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?
    
    // 使用教程地址：
    // https://github.com/Jinkeycode/XloggerSwiftDemo
    
    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey: Any]?) -> Bool {
        
        var jmb = JinkeyMarsBridge()
        jmb.initXlogger(.debug, releaseLevel: .info, path: "/jinkeylog", prefix: "Test")
        jmb.log(.debug, tag: "JinkeyIO", content: "我的公众号是 jinkey-love")
        
        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        
    }

    func applicationWillTerminate(_ application: UIApplication) {
        JinkeyMarsBridge().deinitXlogger()
    }


}

