//
//  ViewController.swift
//  XloggerSwiftDemo
//
//  Created by Jinkey on 2017/1/3.
//  Copyright © 2017年 Jinkey. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // 使用教程地址：
        // https://github.com/Jinkeycode/XloggerSwiftDemo
        
        var logPath = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        print(logPath)
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

