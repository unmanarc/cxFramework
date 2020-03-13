#include "socket_multiplexer.h"
#include <thread>

// TODO: timeout callback

using Ms = std::chrono::milliseconds;

LineID Socket_Multiplexer::connect(const Json::Value &connectionParams, void * multiplexedSocketLocalObject, unsigned int milliseconds)
{
    if (mtLock_multiplexedSocket.try_lock_for(Ms(milliseconds)))
    {
        // send back channel close message, peer must close the connection.
        std::shared_ptr<Socket_Multiplexed_Line> sock = registerLine();
        sock->setLocalObject(multiplexedSocketLocalObject);
        sock->setConnectionParams(connectionParams);

        LineID localLineId = sock->getLineID().localLineId;
        if (    localLineId != NULL_LINE
                && multiplexedSocket->writeU8(MPLX_LINE_CONNECT)
                && sendOnMultiplexedSocket_LineID(localLineId)
                && multiplexedSocket->writeU32(sock->getLocalWindowSize())
                && multiplexedSocket->writeString16(connectionParams.toStyledString())
                )
        {
            mtLock_multiplexedSocket.unlock();
            return localLineId;
        }
        else if (localLineId != NULL_LINE)
        {
            // remove the connection from the connection list.
            stopAndRemoveLine(sock);
            mtLock_multiplexedSocket.unlock();
            return NULL_LINE;
        }

        mtLock_multiplexedSocket.unlock();
        return localLineId;
    }
    return NULL_LINE;
}

void Socket_Multiplexer::client_HandlerConnection_Callback(std::shared_ptr<Socket_Multiplexed_Line> sock)
{
    if (!cbClientConnectAccepted.callbackFunction) return;
    Socket_Base_Stream * ssock = cbClientConnectAccepted.callbackFunction(cbClientConnectAccepted.obj, sock);
    if (ssock) sock->processLine(ssock,this);
    // remove/close the remote connection
    multiplexedSocket_sendLineData(sock->getLineID(),nullptr,0);
    // unregister the connection...
    stopAndRemoveLine(sock);
    if (destroySocketOnClient) delete ssock;
}

void Socket_Multiplexer::client_FailedConnection_Callback(std::shared_ptr<Socket_Multiplexed_Line> sock, eConnectFailedReason reason)
{
    if (!cbClientConnectFailed.callbackFunction) return;
    cbClientConnectFailed.callbackFunction(cbClientConnectFailed.obj, sock, reason);
    stopAndRemoveLine(sock);
}


void clientHandleConnectionFailedThread(sConnectionThreadParams * thrParams)
{
    // here, the callback should initialize the server piece (make the connections, etc)
    ((Socket_Multiplexer *)thrParams->multiPlexer)->client_FailedConnection_Callback(thrParams->chSock, thrParams->reason);
    delete thrParams;
}

void clientHandleConnectionThread(sConnectionThreadParams * thrParams)
{
    // here, the callback should initialize the server piece (make the connections, etc)
    ((Socket_Multiplexer *)thrParams->multiPlexer)->client_HandlerConnection_Callback(thrParams->chSock);
    delete thrParams;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Line_ConnectionAnswer()
{
    bool readen;
    std::string sJMessage;
    Json::Value jAcceptMsg;
    sLineID lineId;
    uint32_t remoteWindowSize;
    eLineAcceptAnswerMSG msg;

    lineId.localLineId = recvFromMultiplexedSocket_LineID(&readen);
    lineId.remoteLineId = recvFromMultiplexedSocket_LineID(&readen);
    remoteWindowSize = multiplexedSocket->readU32(&readen);
    msg = (eLineAcceptAnswerMSG)multiplexedSocket->readU8(&readen);
    sJMessage = multiplexedSocket->readString(&readen, 25); //(max: 32Mb)

    if (readen)
    {
        Json::CharReaderBuilder builder;
        Json::CharReader * reader = builder.newCharReader();
        std::string errors;
        reader->parse(sJMessage.c_str(), sJMessage.c_str() + sJMessage.size(), &(jAcceptMsg), &errors);
        delete reader;

        std::shared_ptr<Socket_Multiplexed_Line> chSock = findLine(lineId.localLineId);

        if (chSock->isValidLine())
        {
            switch (msg)
            {
            case INIT_LINE_ANS_THREADED:
                break;
            case INIT_LINE_ANS_ESTABLISHED:
            {
                sConnectionThreadParams * cntThrParams = new sConnectionThreadParams;
                if (!cntThrParams)
                {
                    // FATAL ERROR: object is waiting for destruction on this thread. will lead to memory corruption/leak
                    stopAndRemoveLine(chSock);
                    multiplexedSocket_sendTermination(lineId);
                    return false;
                }
                cntThrParams->multiPlexer = this;
                cntThrParams->chSock = chSock;

                // Set the remote line ID (neccesary to send anything to them).
                chSock->setLineRemoteID(lineId.remoteLineId);
                // Set remote line window size.
                chSock->setRemoteWindowSize(remoteWindowSize);

                // Start the connection thread

                std::thread(clientHandleConnectionThread,cntThrParams).detach();
                /*
                 * TODO: handle thread fail, but at this point, a failed thread implies program termination
                {
                    // FATAL ERROR: object is waiting for destruction on this thread. will lead to memory corruption/leak
                    stopAndRemoveLine(chSock);
                    multiplexedSocket_sendTermination(lineId);
                    delete cntThrParams;
                    return false;
                }*/
            } break;
            case INIT_LINE_ANS_THREADFAILED:
            case INIT_LINE_ANS_BADPARAMS:
            case INIT_LINE_ANS_FAILED:
            case INIT_LINE_ANS_NOCALLBACK:
            case INIT_LINE_ANS_BADSERVERSOCK:
            case INIT_LINE_ANS_BADLOCALLINE:
            case INIT_LINE_ANS_NOTAUTHORIZED:
            {
                sConnectionThreadParams * cntThrParams = new sConnectionThreadParams;

                if (!cntThrParams)
                {
                    // FATAL ERROR: object is waiting for destruction on this thread. will lead to memory corruption/leak
                    stopAndRemoveLine(chSock);
                    return false;
                }

                cntThrParams->chSock = chSock;
                cntThrParams->multiPlexer = this;

                if (msg == INIT_LINE_ANS_THREADFAILED) cntThrParams->reason = E_CONN_FAILED_ANSTHREAD;
                else if (msg == INIT_LINE_ANS_BADPARAMS) cntThrParams->reason = E_CONN_FAILED_BADPARAMS;
                else if (msg == INIT_LINE_ANS_FAILED) cntThrParams->reason = E_CONN_FAILED;
                else if (msg == INIT_LINE_ANS_NOCALLBACK) cntThrParams->reason = E_CONN_FAILED_NOCALLBACK;
                else if (msg == INIT_LINE_ANS_BADSERVERSOCK) cntThrParams->reason = E_CONN_FAILED_BADSERVERSOCK;
                else if (msg == INIT_LINE_ANS_BADLOCALLINE) cntThrParams->reason = E_CONN_FAILED_BADLOCALLINE;
                else if (msg == INIT_LINE_ANS_NOTAUTHORIZED) cntThrParams->reason = E_CONN_FAILED_NOTAUTHORIZED;

                std::thread(clientHandleConnectionFailedThread, cntThrParams).detach();
                /*
                 * * TODO: handle thread fail, but at this point, a failed thread implies program termination
                {
                    // FATAL ERROR: object is waiting for destruction on this thread. will lead to memory corruption/leak
                    stopAndRemoveLine(chSock);
                    delete cntThrParams;
                    return false;
                }*/
            }break;
            }
        }
        else
        {
            multiplexedSocket_sendTermination(lineId);
        }
        return true;
    }

    return false;
}

