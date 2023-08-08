#include <stream/streamlinereader.h>


StreamLineReader::StreamLineReader(Stream& strm, IOCachPtr linecaches) :
mStrm(strm), mLineCaches(linecaches)
{
    mLineCaches->assign(mLineCaches->size(), '\0');
    mfixedBufferSize = mLineCaches->size();
    mfixedBufferUsedSize = 0;
}

void StreamLineReader::append(char c) {
    if(mfixedBufferUsedSize < mfixedBufferSize - 1) {//如果被内存还没有被使用完
        (*mLineCaches)[mfixedBufferUsedSize++] = c;
        (*mLineCaches)[mfixedBufferUsedSize] = '\0';
    } else {//如果被内存被使用完了
        if(mGrowingBuffers.empty()) {
            assert((*mLineCaches)[mfixedBufferUsedSize]=='\0');
            mGrowingBuffers.assign(mLineCaches->data(), mfixedBufferUsedSize);
        }
        mGrowingBuffers += c;
    }
}

bool StreamLineReader::getLine() {
    mfixedBufferUsedSize = 0;
    mGrowingBuffers.clear();

    for(size_t i=0;; i++) {
        char byte;
        ssize_t n = mStrm.read(&byte, 1);
        if(n < 0) {//读取出错
            return false;
        } else if (n == 0) { //没有读取到字节
            if(i == 0 ) {//读取出错
                return false;
            } else {//读取完毕
                break;
            }
        }
        append(byte);
        if(byte == '\n') break;//遇到换行符，结束
    }
    return true;
}
