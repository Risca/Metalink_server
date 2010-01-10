#include "metalinkconnection.h"
#include "metalinkchat.h"

#include <QtNetwork>

static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 60 * 1000;
static const int PingInterval = 5 * 1000;
static const char SeparatorToken = ' ';

MetaLinkConnection::MetaLinkConnection(QObject *parent)
{

    tcpSocket = new QTcpSocket(this);
    currentState = SendingNick;
    currentDataType = Undefined;

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    connect(this, SIGNAL(receivedNewContactList(QString)), this, SLOT(parseList(QString)));
    connect(this, SIGNAL(incomingChatCommand(QString)), this, SLOT(parseChatCommand(QString)));

    numBytesForCurrentDataType = -1;
    transferTimerId = 0;
    pingTimer.setInterval(PingInterval);
    connect(tcpSocket, SIGNAL(disconnected()), &pingTimer, SLOT(stop()));
    connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
}

void MetaLinkConnection::timerEvent(QTimerEvent *timerEvent)
{
    if (timerEvent->timerId() == transferTimerId) {
        abort();
        killTimer(transferTimerId);
        transferTimerId = 0;
    }
}

QString MetaLinkConnection::nick() const
{
    return myNick;
}

bool MetaLinkConnection::ready()
{
    if(currentState == ReadyForUse)
        return true;

    return false;
}

void MetaLinkConnection::connectToHost(const QString &hostName, quint16 port, OpenMode mode)
{
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostName, port);
}

QString MetaLinkConnection::makeMetaLinkList(QStringList nicks)
{
    if(!nicks.isEmpty()) {
        // Number of nicks
        QString message = QString::number(nicks.size());
        // Each nick is prepended with it's size
        for(int i = 0; i < nicks.size(); i++)
        {
            message += " " + QString::number(nicks.at(i).size()) + " " + nicks.at(i);
        }

        return message;
    } else {
        qDebug() << "Sending empty list??";
        return QString();
    }
}

QStringList MetaLinkConnection::parseMetaLinkList(QString list)
{
    QStringList receivedItems;
    QTextStream message(&list);
    int numberOfItems;
    message >> numberOfItems;
//  qDebug() << "Number of connected clients: " << QString::number(numberOfItems);
    int lengthOfItem;
    while (numberOfItems) {
        message >> lengthOfItem;
        message.read(1);
        receivedItems.append(message.read(lengthOfItem));
        message.read(1);
        numberOfItems--;
    }
    return receivedItems;
}

bool MetaLinkConnection::operator ==(const MetaLinkConnection& other) const
{
    if(nick() == other.nick())
        return true;
    else
        return false;
}

void MetaLinkConnection::processReadyRead()
{
    do {
        if (currentDataType == Undefined) {
            if (!readProtocolHeader())
                return;
        }
        if (!hasEnoughData())
            return;
        processData();
    } while (tcpSocket->bytesAvailable() > 0);
}

void MetaLinkConnection::sendPing()
{
    if (pongTime.elapsed() > PongTimeout) {
        abort();
        return;
    }

    write("PING 1 p");
}

void MetaLinkConnection::sendNick()
{
    QByteArray greeting = nick().toUtf8();
    QByteArray data = "NICK " + QByteArray::number(greeting.size()) + " " + greeting;
    if (tcpSocket->write(data) == data.size()){
//        isGreetingMessageSent = true;
        qDebug() << "Sent nick!";
        currentState = awaitingClientList;
        pingTimer.start();
        pongTime.start();
    }
}

void MetaLinkConnection::sendMessage(DataType type, QString message)
{
    QByteArray data(QByteArray::number(message.size()) + " " + message.toUtf8());

    switch(type) {
    case Message:
        data.prepend("MESSAGE ");
        break;
    case Ping:
//        data.prepend("PING "); //Please use sendPing();
        break;
    case Chat:
        data.prepend("CHAT ");
        break;
    case List:
        data.prepend("LIST ");
        break;
    default:
        break;
    }

    if (write(data) == data.size()) {
        qDebug() << "Sent message: " << data;
    }
}

void MetaLinkConnection::parseList(QString rawList)
{
    qDebug() << "Received client list!\n" << rawList;
    emit incomingContactList(new QStringList(parseMetaLinkList(rawList)));
}

void MetaLinkConnection::startChat(QModelIndex index)
{
    qDebug() << "You double-clicked on: " << index.data();
    startChat(index.data().toString());
}

void MetaLinkConnection::startChat(QString with)
{
    if(!with.isEmpty())
    {
        QByteArray message = "INIT " + QByteArray::number(1);
        message += " " + QByteArray::number(with.size()) + " " + with.toUtf8();
        QByteArray data = "CHAT " + QByteArray::number(message.size()) + " " + message;

        if (tcpSocket->write(data) == data.size()){
            qDebug() << "Sent CHAT INIT!";
        }
    }
}

int MetaLinkConnection::readDataIntoBuffer(int maxSize)
{
    if (maxSize > MaxBufferSize)
        return 0;

    int numBytesBeforeRead = buffer.size();
    if (numBytesBeforeRead == MaxBufferSize) {
        abort();
        return 0;
    }

    while (tcpSocket->bytesAvailable() > 0 && buffer.size() < maxSize) {
        buffer.append(tcpSocket->read(1));
        if (buffer.endsWith(SeparatorToken))
            break;
    }
    return buffer.size() - numBytesBeforeRead;
}

int MetaLinkConnection::dataLengthForCurrentDataType()
{
    if (tcpSocket->bytesAvailable() <= 0 || readDataIntoBuffer() <= 0
            || !buffer.endsWith(SeparatorToken))
        return 0;

    buffer.chop(1);
    int number = buffer.toInt();
    buffer.clear();
    return number;
}

bool MetaLinkConnection::readProtocolHeader()
{
    if (readDataIntoBuffer() <= 0) {
        transferTimerId = startTimer(TransferTimeout);
        return false;
    }

    if (buffer == "PING ") {
        currentDataType = Ping;
    } else if (buffer == "PONG ") {
        currentDataType = Pong;
    } else if (buffer == "MESSAGE ") {
        currentDataType = Message;
    } else if (buffer == "NICK ") {
        currentDataType = Nick;
    } else if (buffer == "LIST ") {
        currentDataType = List;
    } else if (buffer == "CHAT ") {
        qDebug() << "Received a CHAT command";
        currentDataType = Chat;
    } else {
        currentDataType = Undefined;
        abort();
        return false;
    }

    buffer.clear();
    numBytesForCurrentDataType = dataLengthForCurrentDataType();
    return true;
}

bool MetaLinkConnection::hasEnoughData()
{
    if (transferTimerId) {
        QObject::killTimer(transferTimerId);
        transferTimerId = 0;
    }

    if (numBytesForCurrentDataType <= 0)
        numBytesForCurrentDataType = dataLengthForCurrentDataType();

    if (tcpSocket->bytesAvailable() < numBytesForCurrentDataType
            || numBytesForCurrentDataType <= 0) {
        transferTimerId = startTimer(TransferTimeout);
        return false;
    }

    return true;
}

void MetaLinkConnection::processData()
{
    buffer = tcpSocket->read(numBytesForCurrentDataType);
    if (buffer.size() != numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch (currentDataType) {
    case Message:
//        emit newMessage(nick, QString::fromUtf8(buffer));
        break;
    case Ping:
        tcpSocket->write("PONG 1 p");
        break;
    case Pong:
        pongTime.restart();
        break;
    case List:
        emit receivedNewContactList(QString::fromUtf8(buffer));
        break;
    case Chat:
        qDebug() << "Received a CHAT command";
        emit incomingChatCommand(QString::fromUtf8(buffer));
        break;
    default:
        break;
    }

    currentDataType = Undefined;
    numBytesForCurrentDataType = 0;
    buffer.clear();
}

void MetaLinkConnection::acceptChatInvite(MetaLinkChat *chat)
{
    QByteArray message = QByteArray::number(chat->id()) + " ACCEPT";
    QByteArray data = "CHAT " + QByteArray::number(message.size()) + " " + message;

    if (tcpSocket->write(data) == data.size()){
        qDebug() << "Sent CHAT ACCEPT!";
    }
}
