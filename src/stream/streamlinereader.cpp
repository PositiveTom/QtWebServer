#include <stream/streamlinereader.h>


StreamLineReader::StreamLineReader(Stream& strm, IOCachPtr iocaches) :
mStrm(strm), mIOCaches(iocaches)
{

}

bool StreamLineReader::getLine() {
    mGrowingBuffers.clear();


    
}
