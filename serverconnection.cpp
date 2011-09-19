#include "serverconnection.h"

unsigned int ServerConnection::nextID = 0;

ServerConnection::ServerConnection(QObject *parent) :
    Connection(parent),
    _ID(nextID)
#ifdef AUDIO
    ,
    audioStream(new myAudioStream())
#endif
{
    nextID++;
}

void ServerConnection::setNick(QString nick)
{
    _nick = nick;
    emit nickReceived();
}

#ifdef AUDIO
void ServerConnection::startAudioStream(bool start, QString payload)
{
    if (start) {
        if (!payload.isEmpty()) {
            audioStream->setPayload(payload);
        }
        audioStream->setRemoteIP(peerAddress());
        audioStream->start();
    } else {
        audioStream->stop();
    }
}
#endif
