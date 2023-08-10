#include <server/server.h>

Server::Server() {
    mTaskqueue = ThreadPool::getInstance();
    // mTimer = TimerWheel::getInstance();
    // mTimer->startTick(5000);
    timeout = 5; // ms
    mKeepalivemaxcount = 5;
    mMaxMemoryBlocks = 20;
    for(size_t i=0; i<mTaskqueue->WorkerNums(); i++) {
        mMemorypoll.push(std::make_shared<IOCach>(2048, '\0'));
    }
    mCurrentMemoryBlocks = mMemorypoll.size();
}

void Server::createBindToPort(std::string ip, uint16_t port, int backlog, int socket_flags) {
    while(true) {
        /*创建socket*/
        mSrvsock = socket(AF_INET, SOCK_STREAM, socket_flags);
        if(mSrvsock == -1) continue;
        /**
         * fcntl: 对文件描述符号进行控制操作的系统调用
         * F_SETFD: 设置文件描述符标志的操作
        */
        if(fcntl(mSrvsock, F_SETFD, FD_CLOEXEC) == -1) { //fork后，执行子进程时关闭父进程里面的该文件描述符号
            closeSocket(mSrvsock);
            continue;
        }        
        int yes = 1;
        setsockopt(mSrvsock, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const void*>(&yes), sizeof(yes)); //允许多个套接字绑定到同一个端口

        struct sockaddr_in address; //地址转换
        inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        if((bind(mSrvsock, (struct sockaddr*)(&address), sizeof(address)) == -1) || 
           (listen(mSrvsock, backlog) == -1)) {
            closeSocket(mSrvsock);
            continue;
        }
        break;
    }
    LOG(INFO) << "Create, bind and listen succed!";
}

int Server::closeSocket(socket_t sock) {
    return close(sock);
}

/**
 * @brief 从服务器的内存池块中分配一块可用的内存池给子线程
*/
IOCachPtr Server::allocateMemory() {
    std::unique_lock<std::mutex> lock(mMemorylock);
    while(mMemorypoll.empty() && mCurrentMemoryBlocks >= mMaxMemoryBlocks) {
        mCondition.wait(lock);
    }
    IOCachPtr memoryBlock = nullptr;
    if(!mMemorypoll.empty()) {
        memoryBlock = mMemorypoll.front();
        mMemorypoll.pop();
    } else {
        memoryBlock = std::make_shared<IOCach>(2048, '\0');
        ++mCurrentMemoryBlocks;
    }
    return memoryBlock;
}

/**
 * @brief 归还内存给服务器
*/
void Server::deallocateMemory(IOCachPtr memoryblock) {
    std::lock_guard<std::mutex> lock(mMemorylock);
    memoryblock->assign(memoryblock->size(), '\0');
    mMemorypoll.push(memoryblock);
    mCondition.notify_one();
}

/**
 * @brief 处理请求
*/
bool Server::processRequest(Stream& strm, IOCachPtr line_memory) {
    // LOG(WARNING) << line_memory->size();

    StreamLineReader stream_line_reader(strm, line_memory);
    // LOG(INFO) << stream_line_reader.ptr();
    if(!stream_line_reader.getLine()) return false;
    // LOG(INFO) << stream_line_reader.ptr();
    Request req;
    /*解析请求行和头部字段*/
    if(!parseRequestLine(stream_line_reader.ptr(), req) || !read_headers(strm, req.headers, line_memory)) {
        LOG(WARNING) << "header is not full";
        return false;
    }
    // LOG(INFO) << "DEBUG";
    Response res;
    res.version = "HTTP/1.1";
    writeResponse(strm, req, res, line_memory);
    // LOG(WARNING) << line_memory->size();
    if (req.version == "HTTP/1.0" &&
      req.get_header_value("Connection", 0) != "Keep-Alive") {
      // LOG(INFO) << "hello";
      return false;
    }
}

// ssize_t write_headers(Stream &strm, const Headers &headers, IOCachPtr line_memory) {
//   ssize_t write_len = 0;
//   for (const auto &x : headers) {
//     auto len =
//         strm.write_format(line_memory, "%s: %s\r\n", x.first.c_str(), x.second.c_str());
//     if (len < 0) { return len; }
//     write_len += len;
//   }
//   auto len = strm.write("\r\n");
//   if (len < 0) { return len; }
//   write_len += len;
//   return write_len;
// }

// bool write_data(Stream &strm, const char *d, size_t l) {
//   size_t offset = 0;
//   while (offset < l) {
//     auto length = strm.write(d + offset, l - offset);
//     if (length < 0) { return false; }
//     offset += static_cast<size_t>(length);
//   }
//   return true;
// }

bool Server::writeResponse(Stream& strm, const Request& req, Response& res, IOCachPtr line_memory) {
    BufferStream bstrm;
    bstrm.write_format(line_memory, "HTTP/1.1 %d OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello\r\n", 200);
    // write_headers(bstrm, req.headers, line_memory);

    auto &data = bstrm.get_buffer();

    // LOG(INFO) << data;

    handle_EAGAIN([&](){
        return send(strm.socket(), data.data(), data.size(), 0);
    });
    // write_data(bstrm, data.data(), data.size());
}

// bool Server::routing(Request &req, Response &res, Stream &strm) {
//     if(req.method == "GET") {
        
//     }
// }

/**
 * @brief 解析请求行
*/
bool Server::parseRequestLine(const char* s, Request& req) {
  auto len = strlen(s);
  if (len < 2 || s[len - 2] != '\r' || s[len - 1] != '\n') { return false; }
  len -= 2;

  {
    size_t count = 0;

    split(s, s + len, ' ', [&](const char *b, const char *e) {/*把GET / HTTP/1.1 拆分开, 以' '为分隔符*/
      switch (count) {
      case 0: req.method = std::string(b, e); break;/*GET*/
      case 1: req.target = std::string(b, e); break;/*/*/
      case 2: req.version = std::string(b, e); break;/*HTTP/1.1*/
      default: break;
      }
      count++;
    });

    if (count != 3) { return false; }
  }

  static const std::set<std::string> methods{
      "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
      "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};

  if (methods.find(req.method) == methods.end()) { return false; }

  if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0") { return false; }

  {
    // Skip URL fragment /*它是用来标识文档中的特定位置或锚点的。当一个 URL 中包含片段时，浏览器在获取资源后，会尝试自动滚动到 URL 指定的片段所在位置。这样，可以方便地定位到文档的某个特定部分，例如一个标题、段落、图片、表格等等。*/
    for (size_t i = 0; i < req.target.size(); i++) {
      if (req.target[i] == '#') {
        req.target.erase(i);
        break;
      }
    }

    size_t count = 0;

    split(req.target.data(), req.target.data() + req.target.size(), '?',
                  [&](const char *b, const char *e) {
                    switch (count) {
                    case 0:
                      req.path = decode_url(std::string(b, e), false);
                      break;
                    case 1: {
                      if (e - b > 0) {
                        parse_query_text(std::string(b, e), req.params);
                      }
                      break;
                    }
                    default: break;
                    }
                    count++;
                  });/*在url中,?后面的是参数,前面的是路径,拆分url*/

    if (count > 2) { return false; }
  }

  return true;
}


bool read_headers(Stream &strm, Headers &headers, IOCachPtr iocach) {
//   const auto bufsiz = 2048;
//   char buf[bufsiz];
  iocach->assign(iocach->size(), '\0');
  StreamLineReader line_reader(strm, iocach);

  for (;;) {
    if (!line_reader.getLine()) { return false; }

    // Check if the line ends with CRLF./*检查是否有换行符号*/
    auto line_terminator_len = 2;
    if (line_reader.end_with_crlf()) {
      // Blank line indicates end of headers.
      if (line_reader.size() == 2) { break; }/*遇到空行则会跳出循环*/
#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
    } else {
      // Blank line indicates end of headers.
      if (line_reader.size() == 1) { break; }
      line_terminator_len = 1;
    }
#else
    } else {
      continue; // Skip invalid line.
    }
#endif

    // if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }/*如果一行字节数超过最大字节数,报错*/

    // Exclude line terminator
    auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

    parse_header(line_reader.ptr(), end,
                 [&](std::string &&key, std::string &&val) {
                   headers.emplace(std::move(key), std::move(val));
                 });
  }

  return true;
}