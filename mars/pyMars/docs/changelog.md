### changelog

 * changelog
    * 史前阶段:
        * TODO: 添加 task 管理相关方法，并处理 send msg 这种
        * TODO: 植入 py callback 的入口，用来支持 py 写 callback，主要用来处理 push 的 callback
        *  20180629 
        * 已支持 sendmsg 
        * TODO: 处理 push callback ...
        *  20180704 
        * 已支持 push callback
        * TODO: 处理 callback 参数
        * TODO: 写一个 installation.md 和一个 demo.py...
    * 20180704,stage1_v0.1
        * stage1_v0.1a
        * 可以正常 import mars
        * 可以正常 mars.createMars/destroyMars
        * 可以正常 sendMsg
        * 可以正常处理 msg callback，并且用 py 处理cb（90%...
    * 20180704, stage1.1
        * 增加 mars.getPushCallback 和 mars.setPushCallback 方法
        * mars.createMars  支持两种初始化方式. 带初始化 func 和不带初始化 func 两种。
    * 20180705, stage1.2
        * 回调函数完善，带上 cmd_id, task_id, channel_id 等附加参数
    * 20180706, stage1.3
        * 对回调函数加上 gil 锁，解决回调后 stdin 失灵问题。
    * 20180711, stage1.4
        * 彻底从 mars 代码中分离，可以不需要依赖定制版的 mars，直接从官方 mars 融合了
        * 代码整理，去掉多余代码
