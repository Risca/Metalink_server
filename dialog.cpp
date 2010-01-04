#include <QtGui>
#include <QtNetwork>

#include "dialog.h"
#include "ui_dialog.h"

static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 60 * 1000;
static const int PingInterval = 5 * 1000;
static const char SeparatorToken = ' ';

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);
    state = SendingNick;
    currentDataType = Undefined;

    connect(ui->connectButton, SIGNAL(clicked()),
            this, SLOT(connectToHost()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(receivedNewContactList(QString)), this, SLOT(parseList(QString)));
    connect(this, SIGNAL(incomingChatCommand(QString)), this, SLOT(parseChatCommand(QString)));

    setWindowTitle(tr("Fortune Client"));

    numBytesForCurrentDataType = -1;
    transferTimerId = 0;
    pingTimer.setInterval(PingInterval);
    connect(tcpSocket, SIGNAL(disconnected()), &pingTimer, SLOT(stop()));
    connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(startChat(QModelIndex)));
    ui->portLineEdit->setFocus();
}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::nick()
{
    return ui->nickLineEdit->text();
}

void Dialog::activateSendButton()
{
    if(tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        ui->sendButton->setEnabled(true);
    }
}

void Dialog::connectToHost()
{
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(ui->hostnameLineEdit->text(),
                             ui->portLineEdit->text().toInt());
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendNick()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(activateSendButton()));
}

void Dialog::processReadyRead()
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

int Dialog::readDataIntoBuffer(int maxSize)
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

int Dialog::dataLengthForCurrentDataType()
{
    if (tcpSocket->bytesAvailable() <= 0 || readDataIntoBuffer() <= 0
            || !buffer.endsWith(SeparatorToken))
        return 0;

    buffer.chop(1);
    int number = buffer.toInt();
    buffer.clear();
    return number;
}

bool Dialog::readProtocolHeader()
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
        currentDataType = PlainText;
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

bool Dialog::hasEnoughData()
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

void Dialog::sendNick()
{
    QByteArray greeting = nick().toUtf8();
    QByteArray data = "NICK " + QByteArray::number(greeting.size()) + " " + greeting;
    if (tcpSocket->write(data) == data.size()){
//        isGreetingMessageSent = true;
        qDebug() << "Sent nick!";
        state = awaitingClientList;
        pingTimer.start();
        pongTime.start();
    }
}

void Dialog::sendMessage()
{
    QByteArray message = ui->plainTextEdit->toPlainText().toUtf8();
    if(!message.isEmpty())
    {
        QByteArray data = "MESSAGE " + QByteArray::number(message.size()) + " " + message;
        if (tcpSocket->write(data) == data.size()){
            qDebug() << "Sent message!";
        }
    }
}

void Dialog::parseList(QString rawList)
{
    ui->listWidget->clear();
    qDebug() << "Received client list!\n" << rawList;
    QTextStream clientStream(&rawList);
    int numberOfClients;
    clientStream >> numberOfClients;
    qDebug() << "Number of connected clients: " << QString::number(numberOfClients);
    int lengthOfNick;
    QString nick;
    for(int i = 0; i < numberOfClients; i++) {
        clientStream >> lengthOfNick;
        qDebug() << "Length of nick: " << QString::number(lengthOfNick);
        clientStream.read(1);
        nick = clientStream.read(lengthOfNick);
        qDebug() << "Nick: " << nick;
        ui->listWidget->addItem(nick);
        clientStream.read(1);
    }
}
void Dialog::parseChatCommand(QString command)
{
    if (!command.isEmpty())
    {
        QTextStream message(&command);

        qDebug() << "Received the following CHAT command: " << command;

    }
}

void Dialog::startChat(QString with)
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

void Dialog::startChat(QModelIndex index)
{
    qDebug() << "You double-clicked on: " << index.data();
    startChat(index.data().toString());
}

void Dialog::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
}

void Dialog::processData()
{
    buffer = tcpSocket->read(numBytesForCurrentDataType);
    if (buffer.size() != numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch (currentDataType) {
    case PlainText:
//        emit newMessage(nick, QString::fromUtf8(buffer));
        break;
    case Ping:
        qDebug() << "PONG!";
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

void Dialog::sendPing()
{
    if (pongTime.elapsed() > PongTimeout) {
        abort();
        return;
    }

    tcpSocket->write("PING 1 p");
}
