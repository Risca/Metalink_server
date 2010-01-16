#include <QtGui>
#include <QtNetwork>

#include "dialog.h"
#include "metalinkchat.h"
#include "metalinkconnection.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), connection(new MetaLinkConnection)
{
    ui->setupUi(this);

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectToHost()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));

    connection->setNick(ui->nickLineEdit->text());
    connect(ui->nickLineEdit, SIGNAL(textChanged(QString)), connection, SLOT(setNick(QString)));

    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(readyForUse()), this, SLOT(activateSendButton()));
    connect(connection, SIGNAL(incomingContactList(QStringList&)),
            this, SLOT(updateContactList(QStringList&)));
    connect(connection, SIGNAL(incomingChatCommand(QString)), this, SLOT(parseChatCommand(QString)));
    connect(connection, SIGNAL(disconnected()), this, SLOT(disconnectedFromHost()));

    setWindowTitle(tr("MetaLink Client"));

    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), connection, SLOT(startChat(QModelIndex)));
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

void Dialog::connectToHost()
{
    connection->connectToHost(ui->hostnameLineEdit->text(),
                              ui->portLineEdit->text().toInt());
}

void Dialog::activateSendButton()
{
    ui->sendButton->setEnabled(true);
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
                                 .arg(connection->errorString()));
    }
}

void Dialog::sendMessage()
{
    QString message = ui->plainTextEdit->toPlainText();
    message.prepend("0 MESSAGE ");
    connection->send(MetaLinkConnection::Chat, &message);
}

void Dialog::closeChat(MetaLinkChat *chat)
{
    chat->deleteLater();
    chats.removeAll(chat);
}

void Dialog::updateContactList(QStringList &nicks)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(nicks);
}

void Dialog::parseChatCommand(QString command)
{
    if (!command.isEmpty())
    {
        QTextStream message(&command);

        int chatID;
        message >> chatID;
        message.read(1);

        bool found = false;
        int i = 0;
        if (!chats.empty()) {
            do {
                if (chats.at(i)->id() == chatID) {
                    found = true;
                    break;
                }
                i++;
            } while(i < chats.size());
        }

        if(found) {
            chats.at(i)->parseChatCommand(message.readAll());
        } else {
            QString operation;
            message >> operation;
            if(operation == "INVITE") {
                QString *temp = new QString(message.readAll());
                QStringList receivedNicks = MetaLinkConnection::parseMetaLinkList(*temp);

                bool startChat = false;
                // I requested this chat session if
                if(receivedNicks.contains(this->nick())) {
                    startChat = true;
                } else {
                    int ret = QMessageBox::question(this, tr("New chat"),
                                                    tr("The following people wants to contact you:\n") +
                                                    receivedNicks.join("\n"),
                                                    QMessageBox::Ok, QMessageBox::Cancel);

                    if (ret == QMessageBox::Ok) {
                        startChat = true;
                    }
                }

                if(startChat) {
                    MetaLinkChat *chat = new MetaLinkChat(chatID, this->nick(), receivedNicks, connection, this);
                    chats.append(chat);
                    chat->sendCommand(MetaLinkChat::Accept);
                    connect(chat, SIGNAL(leave(MetaLinkChat*)), this, SLOT(closeChat(MetaLinkChat*)));
                    chats.last()->show();
                    chats.last()->raise();
                    chats.last()->activateWindow();
                } else {
                    QString *rejectMessage = new QString(
                            QString::number(chatID) + " REJECT");
                    connection->send(MetaLinkConnection::Chat, rejectMessage);
                }
            } else if (operation == "MESSAGE") {
                QString broadcastMessage = message.readAll();
                // Strip the sendername
                QString from = MetaLinkConnection::parseMetaLinkList(broadcastMessage).first();
                QMessageBox::information(this, from, broadcastMessage, QMessageBox::Ok);
            } else {
                qDebug() << "Received unknown chat ID, and no suited command :S";
            }
        }
    }
}

void Dialog::disconnectedFromHost()
{
    for (int i = 0; i < chats.size(); i++) {
        chats.at(i)->sendCommand(MetaLinkChat::Leave);
    }
    chats.clear();
    ui->listWidget->clear();

    int ret = QMessageBox::question(this, tr("Disconnected to host"),
                                    tr("You were disconnected from host\n") +
                                    tr("Would you like to re-connect?"),
                                    QMessageBox::Yes | QMessageBox::No);

    if(ret==QMessageBox::Yes) {
        connectToHost();
    }
}
