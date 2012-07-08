#include "chatwindow.h"
#include "ui_chatwindow.h"

unsigned int ChatWindow::_nextID = 1;

ChatWindow::ChatWindow(QString starter, QWidget *parent) :
    Chat(parent,_nextID),
    ui(new Ui::ChatWindow)
{
    qDebug() << "New ChatWindow with chatID " << QString::number(_ID);
    _nextID++;
    addParticipant(starter);
    ui->setupUi(this);
    ui->listView->setModel(&_participants);
    ui->historyView->setModel(&_messages);

    connect(ui->destroyButton, SIGNAL(clicked()), this, SLOT(destroyChat()));
    connect(this, SIGNAL(chatEmpty()), this, SLOT(destroyChat()));
    connect(ui->messagePlainTextEdit, SIGNAL(enterPressed()), this, SLOT(sendMessage()));
    connect(ui->sendButton, SIGNAL(clicked()),this,SLOT(sendMessage()));

    // Set nick
    _currentCommand.retranslateCommand(ChatCommand::Message, QStringList("root"));
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::appendMessage(QString from, QString message)
{
    Chat::appendMessage(from, message);
    ui->historyView->scrollToBottom();
}

void ChatWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChatWindow::destroyChat()
{
    if (!participants().isEmpty()) {
        _currentCommand.retranslateCommand(ChatCommand::Leave,
                                           _participants.stringList(),
                                           "DM is ALL-POWERFUL",
                                           _ID);
        emit newCommand();
    }
    ui->destroyButton->setEnabled(false);
}

void ChatWindow::closeEvent(QCloseEvent *e)
{
    if (ui->destroyButton->isEnabled())
        destroyChat();

    e->accept();
}

void ChatWindow::sendMessage()
{
    QString message(ui->messagePlainTextEdit->toPlainText());
    if (!message.isEmpty()) {
        _currentCommand.retranslateCommand(ChatCommand::Message,message);
        emit newCommand();
        ui->messagePlainTextEdit->clear();
        // Manually append our own messages
        appendMessage(_currentCommand.participants().first(),message);
    }
}

void ChatWindow::scrollToBottom()
{
    ui->historyView->scrollToBottom();
}
