# mm_mars_server Intro.

## 安装
 * 编译 protocol buffer 文件到 Python:
   <code>protoc --python_out=./ *.proto</code>
   <code>cd proto</code>
   <code>protoc --python_out=./ *.proto</code>
 * 安装依赖库:
  * <code> pip install -r requirements.txt</code>

## 端口说明:
 * 长连端口：8112
 * 短连端口：8209

## 把端口替换成你自己的:
  * 如果 web server 和 tcp server 分开部署，需要把 WEB_SERVER_HOST 替换成你自己的 host

## 运行:
  * <code> python mm_webserver.py </code>
  * <code> python mm_tcpserver.py </code>