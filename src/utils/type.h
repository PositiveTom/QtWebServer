#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <memorypool/MemoryPool.h>
#ifdef __linux__
    #include <cstring>
#endif


using socket_t = int;
using IOCach = std::vector<char, MemoryPool<char>>;
using IOCachPtr = std::shared_ptr<IOCach>;
using QueueMemoryPool = std::queue<IOCachPtr, std::deque<IOCachPtr>>;
