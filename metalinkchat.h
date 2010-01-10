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
        Undefined
    };

//    MetaLinkChat(int ID, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QString firstParticipant, QTcpSocket *socket, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QStringList firstParticipants, QTcpSocket *socket, QWidget *parent = 0);
    ~MetaLinkChat();

    int id();
    void newParticipantList(QStringList participants);
    void addParticipant(QString nick);
    QStringList participants();
    QString makeChatCommand(ChatCommand commandType, QString &message);
    static ChatCommand determineTypeOfCommand(QString &command);

signals:
    void newMessage(QString message);
    void newChatCommand(QString message);
    void leave(MetaLinkChat*);

public slots:
    void parseChatCommand(QString command);
    void updateNick(QString &nick);
    void sendMessage();

private:
    int myChatID;
    QString myNick;
    Ui::chatDialog *ui;
    QTcpSocket *tcpSocket;

private slots:
    void sendCommand(ChatCommand type, QString *command = new QString);
    void windowClosed();
};

#endif // METALINKCHAT_H
