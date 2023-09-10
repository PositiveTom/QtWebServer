## 此分支不是最新分支，最新分支以及相关代码如下所示：
+ 最新的去Qt的Web服务器分支（为了适配未安装Qt的电脑测试代码）：https://github.com/PositiveTom/QtWebServer/tree/mac
+ 最小堆实现github：https://github.com/PositiveTom/minheap
+ LRUcahce实现github: https://github.com/PositiveTom/LRUCache
+ 前缀树实现github: https://github.com/PositiveTom/Trie
#  环境
推荐ubuntu, MacOS环境待后续替换epoll等linux特有函数即可兼容

#   依赖
```bash
Qt5 (MacOS下需要在CMakeLists.txt文件手动指定Qt5_DIR的路径)
gflags
glog
```

#   编译以及运行
```bash
mkdir build
cd build
cmake ..
make -j12
./bin/QtWebServer

#######浏览器端操作: 输入######
ip : port
```

#   模块

## 1. Proactor模式
+ IO复用：
  + epoll+非阻塞io+ET
+ 主线程：
  + 读取写入io, 放置recv和send函数
+ 工作线程

## 2. reactor模式
+ IO复用：
  + epoll+阻塞io/非阻塞io+ET
+ 工作线程
  + 读取写入io，放置recv和send函数


##  3. Web服务器
参考： 
https://github.com/yhirose/cpp-httplib


1. 采用了 template method, observer, factory method 的设计原则实现了面向接口面向对象的编程, 使得QT登陆界面可以创建自定义的Web服务器

<!-- keep-alive机制, 服务器本身是不知道请求属于同一个客户端, 因此会重复建立TCP连接 -->

##  4. 客户端


##  5. 定时器

### 5.1 时间轮
参考:https://github.com/jsnell/ratas

#### 修改
1. 引入了系统中断来触发一个Tick周期
2. 修改了advance推进逻辑更易理解
3. *空推问题待解决*
4. 时间轮类采用singlton模式, 引入原子操作和双检查锁避免 reorder 问题和 多线程同时竞争占资源问题

## 6. 线程池
参考:https://github.com/progschj/ThreadPool
#### 修改
1. 修改成singleton模式
2. 增加了一些拓展接口
3. 根据cpu核心数创建线程数

## 7. 内存池
参考：https://github.com/cacay/MemoryPool






# telenet 测试注意
```bash
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
lk
```

上面回车发送lk, 网络中的报文数据是 'l', 'k', '\r', '\n' 
'\r':CR回车符号
'\n':LR换行符号
