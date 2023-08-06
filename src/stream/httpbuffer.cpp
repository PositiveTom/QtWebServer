#include <stream/httpbuffer.h>


StreamLineReader::StreamLineReader(Stream& strm, IOCachPtr iocaches) :
mStrm(strm), mIOCaches(iocaches)
{

}