pyMars 文档
----------------

 ***请注意先配置好服务器端***

 * 文档生成
    0. `pip install sphinx_rtd_theme`
    1. `cd docs`
    2. `make html`
    3. `open _build/html/index.html`

 * 背景介绍
    * 缘起  mSolar 之 pyMars  mars 到 python 的 binding
    * 为了改造 IM 而萌生了从现有基于 http request 的`类浏览器`工作模式改到以长连接为重点的工作模式。霸特，直接上 epoll 或者 kqueue 之类的 xx 确实远远超出了我的认知范围。
    * 调研了一圈，发现腾讯去年开源的 mars 还不错，基本就是一个针对国内运营商环境移动互联网的 socket，和在此之上的简单 http，窃喜，初步感觉够用。
    * 官方 sample server 只有 java 的，花了几天时间实现了一般 Python 的，熟悉了一下代码环境，顺手给官方[提了个 pr](https://github.com/Tencent/mars/pull/417)(当然是在获得徐博的首肯的前提下)
    * 由于我大 IM 线有 101 个人（此处有梗...），人手稍微有些紧张，而且考虑到设计一个新的架构总要挖个深一点儿的坑，于是打着方便测试写 UT 单元测试的幌子准备用 Python 调用 Mars，于是乎，从挖到 Mars 本身有用到 boost 开始，入坑了 Boost.Python 一下下，发现不好搞，在大神的建议下换到了 pybind11，树藤摸瓜找到了 binder，发现这是个真坑，生成的代码不分主次，竟然把自带的 boost 库都给导出一份 binding code，无语。
    * 回过头来，参考 fastText 的用法之后还是有些懵逼。
    * 找到一个结合 python dist utils 和 cmake 的 pybind11 模板库， [cmake_example](https://github.com/pybind/cmake_example)，推到所有的尝试，从最简的 helloworld 开始，一行行写 CMakeLists.txt，干掉一堆 bug 之后竟然可以顺利从 Python 中 `import mars` 然后 `mars.createMars()` 启动长短连了。
    * 今天竟然可以 `mars.sendMsg(m.SerializeToString())` 了，不要问我 `m` 从哪里来，那是另一个故事...
    * to be continued...
    
.. toctree::
    :numbered:
    :maxdepth: 2

    server.md
    installation.md
    usage.md
    changelog.md
