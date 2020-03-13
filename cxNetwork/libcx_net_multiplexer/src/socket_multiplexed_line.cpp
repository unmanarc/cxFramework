#include "socket_multiplexed_line.h"
#include "socket_multiplexer.h"
#include "vars.h"
#include <thread>

Socket_Multiplexed_Line::Socket_Multiplexed_Line()
{
    localWindowSize = MUX_LINE_HEAPSIZE; // 512Kb

    lineAttachedSocket = nullptr;
    localObject = nullptr;
    multiPlexer = nullptr;

    remoteUnprocessedFinished = false;
    processLineFinished = false;
    remoteWindowSize = 0;
    remoteUnprocessedBytes = 0;
    localWindowUsedBuffer = 0;

    // TODO: verify this pthread.
    // TODO: verify you can't inject buffer elements during closing
}

Socket_Multiplexed_Line::~Socket_Multiplexed_Line()
{
    sDataBuffer * datab;
    while ((datab=getBufferElement(false))!=nullptr)
    {
        delete datab;
    }

    finalizeProcessor();
}

void Socket_Multiplexed_Line::_lshutdown()
{
    Lock_Mutex_RD lock(rwLock_Vars);
    if (lineAttachedSocket)
        lineAttachedSocket->shutdownSocket();
}

void processLineThread(Socket_Multiplexed_Line * multiplexedLine)
{
    while (multiplexedLine->processBuffer()) {}
}

bool Socket_Multiplexed_Line::processLine(Socket_Base_Stream *lineAttachedSocket, void *multiPlexer)
{
    if (!lineAttachedSocket || !multiPlexer)
    {
        return false;
    }

    Socket_Multiplexer * multiplexedSocket = (Socket_Multiplexer *)multiPlexer;

    {
        // TODO: atomic this vars.
        Lock_Mutex_RW lock(rwLock_Vars);
        this->lineAttachedSocket = lineAttachedSocket;
        this->multiPlexer = multiplexedSocket;
    }

    std::thread x = std::thread(processLineThread,this);

    ////////////////////////////////
    char data[MUX_LINE_SENDBUF+8];

    // TODO: manage shutdowns.
    bool _continue = true;
    while (_continue)
    {
        uint32_t avBytes = getRemoteAvailableBytes();
        uint32_t avlen = avBytes>MUX_LINE_SENDBUF?MUX_LINE_SENDBUF:avBytes;
        int len;
        if ((len = lineAttachedSocket->partialRead(data,avlen))>0)
        {
            Lock_Mutex_RD lock_1(rwLock_Vars);
            Lock_Mutex_RD lock_2(rwLock_LineID);

            if (!multiplexedSocket->multiplexedSocket_sendLineData(lineID, data, len))
            {
                // Can't write into multiplexed socket, get out.

                // TODO: check this
                _lshutdown();// close socket to force processbuffer to leave if is blocking on writeBlock
                addBufferElement(new sDataBuffer); // add empty buffer if it's blocking on getbufferelement
                _continue = false;
            }
            else
            {
                std::unique_lock<std::mutex> lock(mtLock_RemoteProccesedBytes);
                remoteUnprocessedBytes+=len;
            }
        }
        else
        {
            //
            _lshutdown(); // close socket to force processbuffer to leave if is blocking on writeBlock
            addBufferElement(new sDataBuffer); // add empty buffer if it's blocking on getbufferelement
            _continue = false;
        }
    }

    x.join();
    return true;

}

void Socket_Multiplexed_Line::finalizeProcessor()
{
    std::unique_lock<std::mutex> lock(mtLock_ProcessLineFinished);
    processLineFinished = true;
    lock.unlock();
    psigProcessLineFinished.notify_one();
}

void Socket_Multiplexed_Line::waitForProcessLine()
{
    std::unique_lock<std::mutex> lock(mtLock_ProcessLineFinished);
    while (!processLineFinished)
    {
        psigProcessLineFinished.wait(lock);
    }
}

bool Socket_Multiplexed_Line::processBuffer()
{
    bool r=true;
    sDataBuffer * datab = getBufferElement();
    if (!datab->len)
    {
        // TODO: prevent double close
        _lshutdown();
        r=false; // terminate here.
    }
    else
    {
        Lock_Mutex_RD lock(rwLock_Vars);
        if (!lineAttachedSocket->writeBlock(datab->data,datab->len))
        {
            // TODO: if can't write maybe can't read, but anyway, close it.
            _lshutdown();
            r=false; // already terminated.
        }
    }
    delete datab;
    return r;
}

// TODO: prevent micro-chunks flood
bool Socket_Multiplexed_Line::addBufferElement(void *data, uint16_t len)
{
    sDataBuffer * datab = new sDataBuffer;
    if (!datab)
    {
        return false;
    }
    if (!datab->setData(data,len))
    {
        delete datab;
        return false;
    }
    addBufferElement(datab);
    return true;
}

bool Socket_Multiplexed_Line::isValidLine()
{
    Lock_Mutex_RD lock(rwLock_LineID);
    return lineID.localLineId!=NULL_LINE;
}

void Socket_Multiplexed_Line::setLineLocalID(const LineID &value)
{
    Lock_Mutex_RW lock(rwLock_LineID);
    lineID.localLineId = value;
}

void Socket_Multiplexed_Line::setLineRemoteID(const LineID &value)
{
    Lock_Mutex_RW lock(rwLock_LineID);
    lineID.remoteLineId = value;
}

void Socket_Multiplexed_Line::setRemoteWindowSize(const uint32_t &value)
{
    std::unique_lock<std::mutex> lock(mtLock_RemoteProccesedBytes);
    remoteWindowSize = value;
}

bool Socket_Multiplexed_Line::addProcessedBytes(const uint16_t &value)
{
    bool r=true;
    std::unique_lock<std::mutex> lock(mtLock_RemoteProccesedBytes);
    if  (remoteUnprocessedFinished)
    {
        remoteUnprocessedBytes=0;
    }
    else
    {
        if (value>remoteUnprocessedBytes)
        {
            remoteUnprocessedBytes=0;
        }
        else
            remoteUnprocessedBytes-=value;
    }
    lock.unlock();
    psigRemoteProccesedBytesNotFull.notify_one();
    return r;
}

uint32_t Socket_Multiplexed_Line::getLocalWindowSize() const
{
    return localWindowSize;
}

uint32_t Socket_Multiplexed_Line::getRemoteAvailableBytes()
{
    std::unique_lock<std::mutex> lock(mtLock_RemoteProccesedBytes);
    while (remoteUnprocessedBytes>=remoteWindowSize)
    {
        psigRemoteProccesedBytesNotFull.wait(lock);
    }
    return (remoteWindowSize-remoteUnprocessedBytes);
}

void Socket_Multiplexed_Line::resetRemoteAvailableBytes()
{
    std::unique_lock<std::mutex> lock(mtLock_RemoteProccesedBytes);
    remoteUnprocessedBytes=0;
    remoteUnprocessedFinished=true;
    lock.unlock();
    psigRemoteProccesedBytesNotFull.notify_one();
}

sDataBuffer *Socket_Multiplexed_Line::getBufferElement(bool emptyBlocking)
{
    sDataBuffer * dbuf = nullptr;
    if (true)
    {
        std::unique_lock<std::mutex> lock(mtLock_BufferHeap);
        if (emptyBlocking)
        {
            while (dataBuffer.empty())
            {
                psigBufferNotEmpty.wait(lock);
            }
        }
        else
        {
            if (dataBuffer.empty())
                return nullptr;
        }

        // TODO: fragment, because if available bytes in one packet are more than available to send, we have a problem.
        dbuf = dataBuffer.front();
        dataBuffer.pop();

        localWindowUsedBuffer-=dbuf->len;
    }

    psigBufferNotFull.notify_one();

    if (dbuf->len)
    {
        Lock_Mutex_RD lock_1(rwLock_Vars);
        Lock_Mutex_RD lock_2(rwLock_LineID);

        if (multiPlexer)
            ((Socket_Multiplexer *)multiPlexer)->multiplexedSocket_sendReadenBytes(lineID,dbuf->len);
    }

    return dbuf;
}

Json::Value Socket_Multiplexed_Line::getConnectionParams() const
{
    return connectionParams;
}

void Socket_Multiplexed_Line::setConnectionParams(const Json::Value &value)
{
    connectionParams = value;
}

sLineID Socket_Multiplexed_Line::getLineID()
{
    Lock_Mutex_RD lock(rwLock_LineID);
    return lineID;
}

void *Socket_Multiplexed_Line::getLocalObject()
{   
    Lock_Mutex_RD lock(rwLock_LocalObject);
    return localObject;
}

void Socket_Multiplexed_Line::setLocalObject(void *value)
{
    Lock_Mutex_RW lock(rwLock_LocalObject);
    localObject = value;
}

void Socket_Multiplexed_Line::addBufferElement( sDataBuffer * dbuf )
{
    std::unique_lock<std::mutex> lock(mtLock_BufferHeap);

    // TODO restrict incomming packets > to the whole localWindowSize (terminate the connection)
    while (  (dbuf->len+localWindowUsedBuffer) >=  ((localWindowSize*2)+1) )
    {
        psigBufferNotFull.wait(lock);
    }

    dataBuffer.push(dbuf);
    localWindowUsedBuffer+=dbuf->len;

    lock.unlock();
    psigBufferNotEmpty.notify_one();
}
