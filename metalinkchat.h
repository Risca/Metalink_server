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
    MetaLinkChat(QWidget *parent = 0);

signals:

public slots:

private:
    Ui::chatDialog *ui;

};

#endif // METALINKCHAT_H
