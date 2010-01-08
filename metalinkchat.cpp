#include <QtGui>

#include "metalinkchat.h"
#include "ui_chatdialog.h"
/*
MetaLinkChat::MetaLinkChat(int ID, QWidget *parent) :
    QDialog(parent), myChatID(ID), ui(new Ui::chatDialog)
{
    ui->setupUi(this);
}
*/
MetaLinkChat::MetaLinkChat(int ID, QString firstParticipant, QWidget *parent) :
    QDialog(parent), myChatID(ID), ui(new Ui::chatDialog)
{
    ui->setupUi(this);
    if(!firstParticipant.isEmpty())
        ui->listWidget->addItem(firstParticipant);

    
}

MetaLinkChat::MetaLinkChat(int ID, QStringList firstParticipants, QWidget *parent) :
    QDialog(parent), myChatID(ID), ui(new Ui::chatDialog)
{
    ui->setupUi(this);
    if(!firstParticipants.isEmpty())
        ui->listWidget->addItems(firstParticipants);

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
