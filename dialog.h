#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include <QTcpSocket>
#include "metalinkchat.h"
class MetaLinkChat;

static const int MaxBufferSize = 1024000;

class QTcpSocket;

namespace Ui
{
    class Dialog;
}

class Dialog : public QDialog
{
Q_OBJECT
public:
    enum ConnectionState {
        SendingNick,
        awaitingClientList,
        ReadyForUse
    };
    enum DataType {
        PlainText,
        Ping,
        Pong,
        Nick,
        List,
        Chat,
        Undefined
    };

    Dialog(QWidget *parent = 0);
    ~Dialog();

    static QStringList parseMetaLinkList(QString list);

signals:
    void receivedNewContactList(QString rawList);
    void incomingChatCommand(QString command);
    
private slots:
    void activateSendButton();
    void connectToHost();
    void processReadyRead();
    void sendPing();
    void sendNick();
    void displayError(QAbstractSocket::SocketError socketError);
    void sendMessage();
    void parseList(QString rawList);
    void parseChatCommand(QString message);
    void startChat(QString with);
    void closeChat(MetaLinkChat *chat);

private:
    Ui::Dialog *ui;
    QString nick();

    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();
    void acceptChatInvite(MetaLinkChat *chat);

    QTcpSocket *tcpSocket;
    quint16 blockSize;
    QByteArray buffer;
    int numBytesForCurrentDataType;
    ConnectionState state;
    DataType currentDataType;
    QTimer pingTimer;
    QTime pongTime;
    int transferTimerId;
    QList<MetaLinkChat*> chats;

private slots:
    void startChat(QModelIndex index);
};

#endif // DIALOG_H
