### 服务器端配置

 * 服务器端对应的配置方法
    1. 对应的服务器端 code:
        1. `https://code.taou.com/git/dujun/mm_mars_server`
    2. 测试服务器地址:
        1. 106.75.93.248
    3. 长连端口:
        1. 8112
    4. 短连端口:
        1. 8209
    5. 运行:
        * `python mm_webserver.py`
        * `python mm_tcpserver.py`
    6. 注意:
        1. 如果 web server 和 tcp server 分开部署，需要把 WEB_SERVER_HOST 替换成你自己的 host 