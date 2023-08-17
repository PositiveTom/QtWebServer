# 1.运行环境
macOS(macbook air M2,   核总数:8（4性能和4能效)
ubuntu20.04(i5-12400, 12核, 18432kb缓存)

# 2.依赖
```bash
glog
```

# 3.编译运行
```bash
mkdir build
cd build
make -j12
./bin/qtserver
```
# 4.程序框架
## 4.1 网络模式
```mermaid
graph LR
A[对比]
A-->B[Reactor]-->B1[基于待完成的i/o事件,需要调用recv,send函数]
A-->C[Proactor]-->C1[基于已完成的i/o事件,aio系列函数并不是真正的操作系统级别,windows有一套iocp]
```

### (1) 单reactor多线程(线程池)
**src/server/selectserver.cpp**
```mermaid
graph TB
A(开始)
A-->B
B-.->C[[SelectServer::reactorListen&#40&#41]]
subgraph B[单reactor多线程监听,主线程]
    B1[创建服务器,绑定ip地址]
    B1-->B2[设置服务器socket为非阻塞IO]
    B2-->B3[accept从全连接队列取出就绪的文件描述符]
    B3-->B4[/判断取出的套接字sock是否合法/]
    B4--不合法-->B5[关闭或继续]-->B3
    B4--合法-->B6[创建任务,传递sock,添加到工作线程]
end
B6-.->B7
subgraph B7[任务,工作线程]
    B71[申请存储数据的缓冲区内存]-->B72[/select监测sock是否有数据到达/]
    B72--超时-->B73[退出工作线程]
    B72--数据到达-->B74[recv读取数据]
    B74-->B75[处理数据]
    B75-->B76[send发送数据]
    B76-->B77[释放申请的缓冲内存]
    B77-->B73
end
```

**src/server/epollserver.cpp**
```mermaid
graph TB
A(开始)
A-->B
B-.->C[[EpollServer::reactorListen&#40&#41]]
subgraph B[单reactor多线程监听,主线程]
    B1[创建服务器,绑定ip地址]
    B1-->B2[设置服务器socket为非阻塞IO]
    B2-->B3[创建事件表]
    B3-->B4[添加服务器socket到事件表]
    B4-->B5[epoll_wait监听事件表,et模式]
    B5-->B6[/从事件表中取出文件描述符/]
    B6--等于服务器socket-->B7[accept取出套接字,添加到事件表]-->B6
    B6--不等于服务器socket-->B8[申请缓冲区内存]
    B8-->B9
    B9[工作线程]
end
B9-.->B10[s]
```



# 代码debug日志
都位于本地回环测试
参考的测试效果：
|测试平台|IO复用方式,程序框架|1客户端的QPS|10客户端的QPS|100客户端的QPS|1000客户端QPS|10000客户端QPS|
|---|---|---|---|---|---|---|
|macOS|select,非阻塞|5763|10544succed 68failed|1711 susceed, 3318 failed.|360 susceed, 4884 failed|---|
1. 非活跃超时时间 5s-->5ms
此时测试的cpu占用情况：
```bash
Load Avg: 11.63, 8.11, 4.81  CPU usage: 2.14% user, 2.86% sys, 94.99% idle
SharedLibs: 720M resident, 156M data, 55M linkedit.
MemRegions: 255827 total, 4835M resident, 357M private, 1990M shared.
PhysMem: 14G used (1577M wired, 885M compressor), 1974M unused.
VM: 225T vsize, 4283M framework vsize, 0(0) swapins, 0(0) swapouts.
Networks: packets: 3598690/1286M in, 3337340/634M out.
Disks: 439003/11G read, 452278/31G written.
```
|测试平台|IO复用方式,程序框架| 非活跃超时时间      | 1个客户端的QPS(每秒请求数量) |10个客户端的QPS | 100个客户端的QPS |
|---|---| -------- | -------- |-------- |-------- |
|macOS|select,reactor非阻塞IO| 5s      |    1    | 1 | 1 |
|macOS|select,reactor非阻塞IO| 5ms   |    80     | 400| 560succeed 60failed |

2. 发现工作线程执行时间为12ms左右，应该为0.1ms左右才算正常
3. **以上两个原因都不是主要的原因, 出问题的地方在于http解析忽略了"connection != keep-alive"的情况，这个时候应该直接结束掉子线程，关闭tcp连接!!!!! 修改完成之后的测试如下**


| 测试平台 | IO复用方式 | 程序框架 | 检测tcp连接非活跃时间 | 1个客户端的QPS | 10个客户端的QPS | 100个客户端的QPS | 1000个客户端的QPS | 10000个客户端的QPS |
| -------- | ---------- | -------- | --------------------- | -------------- | --------------- | ---------------- | ----------------- | -- |
| MacOS        | 非阻塞io,select          | reactor        | 5s                     | 8140              | 9801 susceed, 36 failed               | 2450 susceed, 4064 failed                | 58 susceed, 18382 failed                 |
|ubuntu20.04|非阻塞io,select|reactor|5s|10787|24854|24305|25547|24491|
|ubuntu20.04|非阻塞io,epoll, ET|单线程|5s|28985|54687|53943|54378|72287|

# 调试工具
##  netstat
用于检测服务器是否正常创建
```bash
netstat -an | grep LISTEN

# -a : 显示所有连接和监听端口，包括那些没有连接的。
# -n : 以数字形式显示地址和端口，而不是尝试解析为主机名和服务名
# | : 管道符号, 用于将一个命令的输出作为另一个命令的输入
# grep LISTEN 使用 grep 工具来过滤出包含字符串 "LISTEN" 的行
```

# 性能测试工具

##  webbench
```bash
./webbench -c 1 -t 1 http://127.0.0.1:8080/
# -c : 客户端的数量
# -t : 运行时间，秒
```