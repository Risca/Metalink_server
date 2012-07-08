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
MetaLinkChat::MetaLinkChat(int ID, QString nick, QString firstParticipant, MetaLinkConnection *connection, QWidget *parent) :
    QDialog(parent), myChatID(ID), myNick(nick), ui(new Ui::chatDialog), myConnection(connection)
{
    ui->setupUi(this);
    if(!firstParticipant.isEmpty())
        ui->listWidget->addItem(firstParticipant);
//    connect(this, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->messagePlainTextEdit, SIGNAL(textChanged()), this, SLOT(saveText()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(this, SIGNAL(newMessage(QString*,QString*)),
            this, SLOT(appendMessage(QString*,QString*)));
}

MetaLinkChat::MetaLinkChat(int ID, QString nick, QStringList firstParticipants, MetaLinkConnection *connection, QWidget *parent) :
    QDialog(parent), myChatID(ID), myNick(nick), ui(new Ui::chatDialog), myConnection(connection)
{
    ui->setupUi(this);
    if(!firstParticipants.isEmpty())
        ui->listWidget->addItems(firstParticipants);
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(this, SIGNAL(newMessage(QString*,QString*)),
            this, SLOT(appendMessage(QString*,QString*)));
}

MetaLinkChat::~MetaLinkChat()
{
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
    case Message:
        message.prepend(QString::number(myChatID) + " MESSAGE ");
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
    } else if (type == "MESSAGE") {
        return MetaLinkChat::Message;
    } else {
        return MetaLinkChat::Undefined;
    }
}

void MetaLinkChat::reject()
{
    sendCommand(MetaLinkChat::Leave);
    emit leave(this);

    QDialog::reject();
}

void MetaLinkChat::parseChatCommand(QString command)
{
    ChatCommand currentChatCommand = determineTypeOfCommand(command);
    QStringList receivedNicks;

    qDebug() << "Chat ID: " << QString::number(myChatID);
    switch (currentChatCommand) {
    case Init:
        break;
    case Invite:
        break;
    case List:
        receivedNicks = MetaLinkConnection::parseMetaLinkList(command);
        qDebug() << "Received chat list: " << receivedNicks;
        newParticipantList(receivedNicks);
        break;
    case Accept:
        break;
    case Reject:
        ui->historyPlainTextEdit->appendPlainText(command + tr(" did not accept the invitation"));
        break;
    case Leave:
        QMessageBox::warning(this, tr("Message"), command, QMessageBox::Ok);
        emit leave(this);
        break;
    case Message:
        receivedNicks = MetaLinkConnection::parseMetaLinkList(command);
        emit newMessage(&receivedNicks.first(), &command);
        qDebug() << "Received a Message from: " << receivedNicks;
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
    QString text = ui->messagePlainTextEdit->toPlainText();
    qDebug() << "Read " << text;
    if(!text.isEmpty()) {
        sendCommand(Message, &text);
    }
    ui->messagePlainTextEdit->clear();
}

void MetaLinkChat::sendCommand(ChatCommand type, QString *command)
{
    makeChatCommand(type, *command);
    qDebug() << "Command " << *command;
    myConnection->send(MetaLinkConnection::Chat, command);
}

void MetaLinkChat::appendMessage(QString *from, QString *message)
{
    if(participants().contains(*from) && !message->isEmpty()) {
        ui->historyPlainTextEdit->appendPlainText(*from + ":\n  " + *message);
    }
}

void MetaLinkChat::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void MetaLinkChat::dropEvent(QDropEvent *event)
{
    QString inviteList =
            MetaLinkConnection::makeMetaLinkList(event->mimeData()->text());
    sendCommand(MetaLinkChat::Invite, &inviteList);

    event->acceptProposedAction();
}
