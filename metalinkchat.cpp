#include <QtGui>

#include "metalinkchat.h"
#include "metalinkconnection.h"
#include "ui_chatdialog.h"
/*
MetaLinkChat::MetaLinkChat(int ID, QWidget *parent) :
    QDialog(parent), myChatID(ID), ui(new Ui::chatDialog)
{
    ui->setupUi(this);
}
*/
MetaLinkChat::MetaLinkChat(int ID, QString nick, QString firstParticipant, QTcpSocket *socket, QWidget *parent) :
    QDialog(parent), myChatID(ID), myNick(nick), ui(new Ui::chatDialog), tcpSocket(socket)
{
    ui->setupUi(this);
    if(!firstParticipant.isEmpty())
        ui->listWidget->addItem(firstParticipant);
    connect(this, SIGNAL(rejected()), this, SLOT(windowClosed()));
}

MetaLinkChat::MetaLinkChat(int ID, QString nick, QStringList firstParticipants, QTcpSocket *socket, QWidget *parent) :
    QDialog(parent), myChatID(ID), myNick(nick), ui(new Ui::chatDialog), tcpSocket(socket)
{
    ui->setupUi(this);
    if(!firstParticipants.isEmpty())
        ui->listWidget->addItems(firstParticipants);
    connect(this, SIGNAL(rejected()), this, SLOT(windowClosed()));
}

MetaLinkChat::~MetaLinkChat()
{
    sendCommand(MetaLinkChat::Leave);
}

int MetaLinkChat::id()
{
    return myChatID;
}

void MetaLinkChat::newParticipantList(QStringList participants)
{
    ui->listWidget->clear();
    for (int i=0; i < participants.size(); i++) {
        addParticipant(participants.at(i));
    }
}

void MetaLinkChat::addParticipant(QString nick)
{
    ui->listWidget->addItem(nick);
}
QStringList MetaLinkChat::participants()
{
    QStringList nicks;
    for (int i=0; i < ui->listWidget->count(); i++) {
        nicks << ui->listWidget->item(i)->text(); //Ugly, I know!
    }
    return nicks;
}

QString MetaLinkChat::makeChatCommand(ChatCommand commandType, QString &message)
{
    switch (commandType) {
    case Init:
        message.prepend("0 INIT ");
        break;
    case Invite:
        message.prepend(QString::number(myChatID) + " INVITE ");
        break;
    case List:
        message.prepend(QString::number(myChatID) + " LIST ");
        break;
    case Accept:
        message.prepend(QString::number(myChatID) + " ACCEPT ");
        break;
    case Reject:
        message.prepend(QString::number(myChatID) + " REJECT ");
        break;
    case Leave:
        message.prepend(QString::number(myChatID) + " LEAVE ");
        break;
    default:
        break;
    }
    return message;
}

MetaLinkChat::ChatCommand MetaLinkChat::determineTypeOfCommand(QString &command)
{
    QString type;
    QTextStream message(&command);
    message >> type;
    command = message.readAll();

    if (type == "INIT") {
        return MetaLinkChat::Init;
    } else if (type == "INVITE") {
        return MetaLinkChat::Invite;
    } else if (type == "LIST") {
        return MetaLinkChat::List;
    } else if (type == "ACCEPT") {
        return MetaLinkChat::Accept;
    } else if (type == "REJECT") {
        return MetaLinkChat::Reject;
    } else if (type == "LEAVE") {
        return MetaLinkChat::Leave;
    } else {
        return MetaLinkChat::Undefined;
    }
}

void MetaLinkChat::parseChatCommand(QString command)
{
    ChatCommand currentChatCommand = determineTypeOfCommand(command);
    QStringList ListOfNicks;

    switch (currentChatCommand) {
    case Init:
        break;
    case Invite:
        break;
    case List:
        ListOfNicks = MetaLinkConnection::parseMetaLinkList(command);
        newParticipantList(ListOfNicks);
        break;
    case Accept:
        break;
    case Reject:
        break;
    case Leave:
        break;
    case Undefined:
        qDebug() << "Undefined CHAT command";
        break;
    default:
        break;
    }
}

void MetaLinkChat::updateNick(QString &nick)
{
    myNick = nick;
}

void MetaLinkChat::sendMessage()
{
    //Fix!
}

void MetaLinkChat::sendCommand(ChatCommand type, QString *command)
{
    makeChatCommand(type, *command);
    QByteArray data = "CHAT " + QByteArray::number(command->size()) + " " + command->toUtf8();

    if (tcpSocket->write(data) == data.size()){
        qDebug() << "Sent CHAT command: " << *command;
    }
    emit newChatCommand(*command);
}

void MetaLinkChat::windowClosed()
{
    sendCommand(MetaLinkChat::Leave);
    emit leave(this);
}
