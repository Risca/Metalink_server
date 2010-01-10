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

    connect(ui->connectButton, SIGNAL(clicked()),
            this, SLOT(connectToHost()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(connected()), this, SLOT(activateSendButton()));
    connect(connection, SIGNAL(connected()), this, SLOT(sendNick()));
    connect(connection, SIGNAL(incomingContactList(QStringList*)),
            this, SLOT(updateContactList(QStringList&)));

    setWindowTitle(tr("MetaLink Client"));

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

void Dialog::connectToHost()
{
    connection->connectToHost(ui->hostnameLineEdit->text(),
                              ui->portLineEdit->text().toInt());
}

void Dialog::activateSendButton()
{
    if(connection->state() == QTcpSocket::ConnectedState)
    {
        ui->sendButton->setEnabled(true);
    }
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

void Dialog::closeChat(MetaLinkChat *chat)
{
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
        qDebug() << "Received the following CHAT command: " << command;

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
                QStringList receivedNicks = MetaLinkConnection::parseMetaLinkList(message.readAll());
                // I requested this chat session if
                if(receivedNicks.contains(this->nick())) {
                    MetaLinkChat *chat = new MetaLinkChat(chatID, this->nick(), receivedNicks,
                                                          connection, this);
                    chats.append(chat);
                    connect(chat, SIGNAL(leave(MetaLinkChat*)), this, SLOT(closeChat(MetaLinkChat*)));
                } else {
                    int ret = QMessageBox::question(this, tr("New chat"),
                                                    tr("The following people wants to contact you:\n") +
                                                    receivedNicks.join("\n"),
                                                    QMessageBox::Ok, QMessageBox::Cancel);

                    if (ret == QMessageBox::Ok) {
                        MetaLinkChat *chat = new MetaLinkChat(chatID, this->nick(), receivedNicks,
                                                              connection, this);
                        chats.append(chat);
                        connection->acceptChatInvite(chat);
                        connect(chat, SIGNAL(leave(MetaLinkChat*)), this, SLOT(closeChat(MetaLinkChat*)));
                    }
                }

                chats.last()->show();
                chats.last()->raise();
                chats.last()->activateWindow();
            } else {
                qDebug() << "Received unknown chat ID, but no invite command :S";
            }
        }
    }
}
