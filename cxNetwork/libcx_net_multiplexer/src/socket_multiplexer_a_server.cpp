#include "socket_multiplexer.h"
#include <thread>

bool Socket_Multiplexer::multiplexedSocket_sendLineConnectionAnswer(const sLineID &lineId, const eLineAcceptAnswerMSG &msg, const uint32_t &localLineWindow)
{
    Json::Value x;
    return multiplexedSocket_sendLineConnectionAnswer(lineId, msg, x, localLineWindow);
}

bool Socket_Multiplexer::multiplexedSocket_sendLineConnectionAnswer(const sLineID & lineId, const eLineAcceptAnswerMSG &msg, const Json::Value &answerValue, const uint32_t & localLineWindow)
{
    if (noSendData) return false;
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);

    if (!multiplexedSocket->writeU8(MPLX_LINE_CONNECT_ANS))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.remoteLineId))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.localLineId))
    {
        return false;
    }
    if (!multiplexedSocket->writeU32(localLineWindow))
    {
        return false;
    }
    if (!multiplexedSocket->writeU8(msg))
    {
        return false;
    }
    if (!multiplexedSocket->writeString32(answerValue.toStyledString(),JSON_MAX_DATA))
    {
        return false;
    }
    return true;
}

void serverAcceptConnectionThread(sServerLineInitThreadParams * thrParams)
{
    // here, the callback should initialize the server piece (make the connections, etc)
    static_cast<Socket_Multiplexer *>(thrParams->multiPlexer)->server_AcceptConnection_Callback(thrParams->lineID, thrParams->remoteWindowSize,thrParams->jConnectionParams);
    delete thrParams;
}

void Socket_Multiplexer::server_AcceptConnection_Callback(sLineID lineId,const uint32_t &remoteWindowSize, const Json::Value &connectionParams)
{
    std::shared_ptr<Socket_Multiplexed_Line> sock = registerLine();

    if (!sock->isValidLine())
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,INIT_LINE_ANS_BADLOCALLINE);
        return;
    }

    if (!cbServerConnectAcceptor.callbackFunction)
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,INIT_LINE_ANS_NOCALLBACK);
        stopAndRemoveLine(sock);
        return;
    }

    // HERE WE CALL THE CALLBACK FUNCTION!!! -> should not take to long or block to work properly.
    Socket_Base_Stream * ssock = cbServerConnectAcceptor.callbackFunction(cbServerConnectAcceptor.obj,sock->getLineID().localLineId,connectionParams);

    if (!ssock)
    {
        multiplexedSocket_sendLineConnectionAnswer(lineId,INIT_LINE_ANS_BADSERVERSOCK);
        stopAndRemoveLine(sock);
        return;
    }

    sock->setRemoteWindowSize(remoteWindowSize);
    sock->setLineRemoteID(lineId.remoteLineId);
    lineId = sock->getLineID();
    multiplexedSocket_sendLineConnectionAnswer(lineId,INIT_LINE_ANS_ESTABLISHED, sock->getLocalWindowSize());

    sock->processLine(ssock,this);

    // TODO: prevent double delete??

    // remove/close the remote connection
    multiplexedSocket_sendLineData(lineId,nullptr,0);
    // unregister the connection...
    stopAndRemoveLine(sock);

    if (cbServerConnectionFinished.callbackFunction) cbServerConnectionFinished.callbackFunction(cbServerConnectionFinished.obj, lineId.localLineId, ssock);

    if (destroySocketOnServer) delete ssock;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Line_Connect(bool authorized)
{
    bool readen;
    std::string connectionParamsStr;

    sServerLineInitThreadParams * thrParams = new sServerLineInitThreadParams;
    if (!thrParams)
    {
        return false; // MEMORY PROBLEM => CLOSE THE LINE...
    }
    thrParams->multiPlexer = this;

    thrParams->lineID.remoteLineId = recvFromMultiplexedSocket_LineID(&readen);
    thrParams->remoteWindowSize = multiplexedSocket->readU32(&readen);
    connectionParamsStr = multiplexedSocket->readString(&readen, 25);

    if (readen)
    {

        if (authorized)
        {
            Json::CharReaderBuilder builder;
            Json::CharReader * reader = builder.newCharReader();
            std::string errors;

            bool parsingSuccessful = reader->parse(connectionParamsStr.c_str(), connectionParamsStr.c_str() + connectionParamsStr.size(), &(thrParams->jConnectionParams), &errors);
            delete reader;

            if ( parsingSuccessful )
            {
                std::thread(serverAcceptConnectionThread, thrParams).detach();
                multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, INIT_LINE_ANS_THREADED);
                return true;
                // TODO: handle thread fail as follows:
                //    multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, INIT_LINE_ANS_THREADFAILED);

            }
            else
                multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, INIT_LINE_ANS_BADPARAMS);
        }
        else
            multiplexedSocket_sendLineConnectionAnswer(thrParams->lineID, INIT_LINE_ANS_NOTAUTHORIZED);
    }

    delete thrParams;

    return false;
}

