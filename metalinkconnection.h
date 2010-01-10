#ifndef METALINKCONNECTION_H
#define METALINKCONNECTION_H

#include <QTcpSocket>
#include <QStringList>
#include <QHostAddress>
//#include <QString>
#include <QTcpSocket>
#include <QTime>
#include <QTimer>
#include <QtGui>

class MetaLinkChat;

static const int MaxBufferSize = 1024000;

class MetaLinkConnection : public QTcpSocket
{
    Q_OBJECT

public:
    enum ConnectionState {
        SendingNick,
        awaitingClientList,
        ReadyForUse
    };
    enum DataType {
        Message,
        Ping,
        Pong,
        Nick,
        List,
        Chat,
        Undefined
    };

    MetaLinkConnection(QObject *parent = 0);

    QString nick() const;
    bool ready();
    void connectToHost(const QString &hostName, quint16 port, OpenMode mode = ReadWrite);

    static QString makeMetaLinkList(QStringList nicks);
    static QStringList parseMetaLinkList(QString list);
    bool operator==(const MetaLinkConnection& other) const;

public slots:
    void sendMessage(DataType type, QString message);
    void acceptChatInvite(MetaLinkChat *chat);

signals:
    void receivedNewContactList(QString);
    void incomingContactList(QStringList*);
    void incomingChatCommand(QString);
    void readyForUse();

protected:
     void timerEvent(QTimerEvent *timerEvent);

private slots:
    void processReadyRead();
    void sendPing();
    void sendNick();
    void sendMessage(QString message);
    void parseList(QString rawList);
    void parseChatCommand(QString message);
    void startChat(QModelIndex index);
    void startChat(QString with);

private:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();

    QTcpSocket *tcpSocket;
    quint16 blockSize;
    QByteArray buffer;
    int numBytesForCurrentDataType;
    ConnectionState currentState;
    DataType currentDataType;
    QTimer pingTimer;
    QTime pongTime;
    int transferTimerId;
    QString myNick;
};

#endif // METALINKCONNECTION_H
