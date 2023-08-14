# epoll
+ epoll_create中的size并不会固定事件表的大小，事件表是一个动态变化！！！如果添加的套接字多，内部会自动扩容！！！
+ epoll_create中的size与epoll_event的size没有直接关系，epoll_event的size决定了一次循环能够处理的套接字个数！！！

## 常见API
### eventfd()
用于进程间通信，通过套接字实现

### epoll_wait()
对于高并发，其timeout参数的设置？tiny-webserver为-1

### epoll_event
evnets参数
EPOLLRDHUP:客户端使用了close函数关闭套接字，服务端就会触发这个机制
EPOLLHUP:连接状态发生了异常或者挂起

### epoll_ctl
EPOLL_CTL_MOD: 适合proactor使用，为什么？当处理完数据后，工作线程使用这个标识，修改套接字为可写，此时主线程再次循环会触发这个套接字，反映到epoll_wait的触发上

### accept
阻塞性取决于文件描述符！！！
+ 阻塞io时
  + 没有tcp连接时，调用accept，errno返回EAGAIN