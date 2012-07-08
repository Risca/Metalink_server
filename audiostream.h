#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include <mediastreamer2/mediastream.h>
#include <ortp/ortp.h>
#include <mediastreamer2/msrtp.h>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QHostAddress>

class myAudioStream
{
public:
    myAudioStream();
    ~myAudioStream();

    void start();
    void stop();

    QString getStatus();
    const QString & payload()
    { return _payload; }
    void setPayload(QString payload)
    { _payload = payload; }
    void changePayload(const QString &payload);
    static QStringList payloads();
    void setRemoteIP(QHostAddress remoteIP)
    { _remoteIP = remoteIP; }


private:
    MSSndCard * init_card();
    RtpProfile * init_rtp_profile();
    bool init_stream();
    bool init_filters();
    bool link_filters();
    bool start_ticker();
    void stop_ticker();
    void unlink_filters();
    void destroy_filters();

    AudioStream *stream;
    static MSSndCard *sndcard_capture;
    static RtpProfile *rtp_profile;
    MSTicker *ticker;
    bool filtersLinked;
    static bool initialized;
    QString _payload;
    QHostAddress _remoteIP;
};

#endif // AUDIOSTREAM_H
