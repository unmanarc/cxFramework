#include "bridge_thread.h"

Bridge_Thread::Bridge_Thread()
{
    block_fwd = nullptr;
    block_rev = nullptr;
    setBlockSize(8192);
}

Bridge_Thread::~Bridge_Thread()
{
    delete [] block_fwd;
    delete [] block_rev;
}

void Bridge_Thread::setStreamSockets(Socket_Base_Stream *src, Socket_Base_Stream *dst)
{
    this->src = src;
    this->dst = dst;
}

int Bridge_Thread::processPipeFWD()
{
    return simpleProcessPipe(true);
}

int Bridge_Thread::processPipeREV()
{
    return simpleProcessPipe(false);
}

bool Bridge_Thread::startPipeSync()
{
    return true;
}

void Bridge_Thread::setBlockSize(const uint32_t & value)
{
    if (block_fwd) delete [] block_fwd;
    if (block_rev) delete [] block_rev;
    blockSize = value;
    block_fwd = new char[value];
    block_rev = new char[value];
}

bool Bridge_Thread::writeBlockL(const void *data, const uint32_t & datalen, bool fwd)
{
    std::lock_guard<std::mutex> lock(fwd?mt_fwd:mt_rev);
    Socket_Base_Stream *dstX=fwd?dst:src;
    return dstX->writeBlock(data,datalen);
}

int Bridge_Thread::simpleProcessPipe(bool fwd)
{
    char * curBlock = fwd?block_fwd:block_rev;

    int bytesReceived;
    if ((bytesReceived=partialReadL(curBlock,blockSize,fwd))>0)
    {
        if (!writeBlockL(curBlock,bytesReceived,fwd)) return -2;
        // Update Counters:
        return bytesReceived;
    }
    return -1;
}

int Bridge_Thread::partialReadL(void *data, const uint32_t &datalen, bool fwd)
{
    return (fwd?src:dst)->partialRead(data,datalen);
}
