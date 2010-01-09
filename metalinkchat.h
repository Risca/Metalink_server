#ifndef METALINKCHAT_H
#define METALINKCHAT_H

#include <QtGui>

namespace Ui
{
    class chatDialog;
}

class MetaLinkChat : public QDialog
{
Q_OBJECT
public:
//    MetaLinkChat(int ID, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QString firstParticipant, QWidget *parent = 0);
    MetaLinkChat(int ID, QString nick, QStringList firstParticipants, QWidget *parent = 0);

    int id();
    void newParticipantList(QStringList participants);
    void addParticipant(QString nick);
signals:

public slots:
    void parseChatCommand(QString command);
    void updateNick(QString &nick);

private:
    int myChatID;
    QString myNick;
    Ui::chatDialog *ui;

};

#endif // METALINKCHAT_H
