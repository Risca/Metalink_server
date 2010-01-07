#include <QtGui>

#include "metalinkchat.h"
#include "ui_chatdialog.h"

MetaLinkChat::MetaLinkChat(QWidget *parent) :
    QDialog(parent), ui(new Ui::chatDialog)
{
    ui->setupUi(this);
}
