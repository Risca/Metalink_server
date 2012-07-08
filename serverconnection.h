#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include "connection.h"
#ifdef AUDIO
#include "audiostream.h"
#endif

class ServerConnection : public Connection
{
Q_OBJECT
public:
    explicit ServerConnection(QObject *parent = 0);

    int id() { return _ID; }
    // Reimplemented to use nickReceived() signal
    void setNick(QString nick);
#ifdef AUDIO
    void startAudioStream(bool, QString payload);
    void changeAudioStreamPayload(QString payload)
    { audioStream->changePayload(payload); }
#endif

signals:
    void nickReceived();

private:
    unsigned int _ID;
    static unsigned int nextID;
#ifdef AUDIO
    myAudioStream *audioStream;
#endif
};

#endif // SERVERCONNECTION_H
