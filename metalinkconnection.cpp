#include "metalinkconnection.h"
#include "metalinkchat.h"

#include <QtNetwork>

static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 60 * 1000;
static const int PingInterval = 5 * 1000;
static const char SeparatorToken = ' ';

MetaLinkConnection::MetaLinkConnection(QObject *parent) :
        QTcpSocket(parent), myNick(new QString)
{

    tcpSocket = new QTcpSocket(this);
    currentState = SendingNick;
    currentDataType = Undefined;

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendNick()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processReadyRead()));

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

QString & MetaLinkConnection::nick() const
{
    return *myNick;
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
    tcpSocket->connectToHost(hostName, port, mode);
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

QStringList MetaLinkConnection::parseMetaLinkList(QString &list)
{
    QStringList receivedItems;
    QTextStream message(&list);
    int numberOfItems;
    message >> numberOfItems;
    int lengthOfItem;
    while (numberOfItems) {
        message >> lengthOfItem;
        message.read(1);
        receivedItems.append(message.read(lengthOfItem));
        message.read(1);
        numberOfItems--;
    }
    //Rest is not part of MetaLinkList
    list = message.readAll();
    return receivedItems;
}

bool MetaLinkConnection::operator ==(const MetaLinkConnection& other) const
{
    if(nick() == other.nick())
        return true;
    else
        return false;
}

void MetaLinkConnection::setNick(QString nick)
{
    *myNick = nick;
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

    send(Ping);
}

void MetaLinkConnection::sendNick()
{
    send(Nick, myNick);
    currentState = AwaitingClientList;
    pingTimer.start();
    pongTime.start();
}

void MetaLinkConnection::send(DataType type, QString *message)
{
    QByteArray data(QByteArray::number(message->size()) + " " + message->toUtf8());

    switch(type) {
    case Ping:
        data.prepend("PING ");
        break;
    case Pong:
        data.prepend("PONG ");
        break;
    case Nick:
        data.prepend("NICK ");
        break;
    case List:
        data.prepend("LIST ");
        break;
    case Chat:
        data.prepend("CHAT ");
        break;
    default:
        break;
    }

    if (tcpSocket->write(data) == data.size()) {
        if(type!=Ping && type!=Pong) { //Filter out some pings and pongs
            qDebug() << "Sent message: " << data;
        }
    }
}

void MetaLinkConnection::startChat(QModelIndex index)
{
    startChat(index.data().toString());
}

void MetaLinkConnection::startChat(QString with)
{
    if(!with.isEmpty())
    {
        QString message = "0 INIT " + QString::number(1);
        message += " " + QString::number(with.size()) + " " + with;
        send(Chat, &message);
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
    } else if (buffer == "NICK ") {
        currentDataType = Nick;
    } else if (buffer == "LIST ") {
        currentDataType = List;
    } else if (buffer == "CHAT ") {
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

    QString temp;
    QStringList receivedNicks;
    switch (currentDataType) {
    case Ping:
        send(Pong);
        break;
    case Pong:
        pongTime.restart();
        break;
    case List:
        if(currentState==AwaitingClientList) {
            currentState=ReadyForUse;
            emit readyForUse();
        }
        temp = QString::fromUtf8(buffer);
        receivedNicks = parseMetaLinkList(temp);
        qDebug() << "Received list: " << receivedNicks;
        emit incomingContactList(receivedNicks);
        break;
    case Chat:
        emit incomingChatCommand(QString::fromUtf8(buffer));
        break;
    default:
        break;
    }

    currentDataType = Undefined;
    numBytesForCurrentDataType = 0;
    buffer.clear();
}
