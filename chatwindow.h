#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "chat.h"

namespace Ui {
    class ChatWindow;
}

class ChatWindow : public Chat
{
Q_OBJECT
public:
    explicit ChatWindow(QString starter, QWidget *parent = 0);
    ~ChatWindow();

    void appendMessage(QString from, QString message);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ChatWindow *ui;
    static unsigned int _nextID;
    void closeEvent(QCloseEvent *);

private slots:
    void destroyChat();
    void sendMessage();
    void scrollToBottom();
};

#endif // CHATWINDOW_H
