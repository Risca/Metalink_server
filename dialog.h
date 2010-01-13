#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include <QTcpSocket>
//#include "metalinkchat.h"
class MetaLinkChat;
class MetaLinkConnection;

namespace Ui
{
    class Dialog;
}

class Dialog : public QDialog
{
Q_OBJECT
public:
    Dialog(QWidget *parent = 0);
    ~Dialog();
    
public slots:
    void connectToHost();
    void activateSendButton();
    void displayError(QAbstractSocket::SocketError socketError);
    void sendMessage();
    void closeChat(MetaLinkChat *chat);
    void updateContactList(QStringList &nicks);
    void parseChatCommand(QString command);

private:
    Ui::Dialog *ui;
    QString nick();
    QList<MetaLinkChat*> chats;
    MetaLinkConnection *connection;


private slots:
    void disconnectedFromHost();

};

#endif // DIALOG_H
