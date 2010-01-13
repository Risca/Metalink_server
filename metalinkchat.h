#ifndef METALINKCHAT_H
#define METALINKCHAT_H

#include <QtGui>
#include "dialog.h"

namespace Ui
{
    class chatDialog;
}

class QTcpSocket;

class MetaLinkChat : public QDialog
{
Q_OBJECT
public:
    enum ChatCommand {
        Init,
        Invite,
        List,
        Accept,
        Reject,
        Leave,
        Message,
        Undefined
    };

//    MetaLinkChat(int ID, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QString firstParticipant, MetaLinkConnection *connection, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QStringList firstParticipants, MetaLinkConnection *connection, QWidget *parent = 0);
    ~MetaLinkChat();

    int id();
    void newParticipantList(QStringList participants);
    void addParticipant(QString nick);
    QStringList participants();
    QString makeChatCommand(ChatCommand commandType, QString &message);
    static ChatCommand determineTypeOfCommand(QString &command);
    void reject();

signals:
    void newMessage(QString *from, QString *message);
    void leave(MetaLinkChat*);

public slots:
    void parseChatCommand(QString command);
    void updateNick(QString &nick);
    void sendMessage();
    void sendCommand(ChatCommand type, QString *command = new QString);
    void appendMessage(QString *from, QString *message);

private:
    int myChatID;
    QString myNick;
    Ui::chatDialog *ui;
    QTcpSocket *tcpSocket;
    MetaLinkConnection *myConnection;

};

#endif // METALINKCHAT_H
