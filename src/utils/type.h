#pragma once

#include <vector>
#include <memorypool/MemoryPool.h>

using socket_t = int;
using IOCachPtr = std::vector<char, MemoryPool<char>>*;

