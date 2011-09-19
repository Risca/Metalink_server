#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    Chat(parent,0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView->setModel(&_participants);
    ui->messageView->setModel(&_messages);

    _currentCommand.retranslateCommand(ChatCommand::Message, QStringList("root"));

    connect(ui->plainTextEdit,SIGNAL(enterPressed()),this,SLOT(sendMessage()));
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendMessage()));
    connect(this,SIGNAL(dataChanged()),this,SLOT(scrollToBottom()));

#ifdef AUDIO
    QStringList payloads;
    payloads << "speex" << "iLBC";
    setupAudioToolbar(payloads);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
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

void MainWindow::enableChat(bool y)
{
    ui->plainTextEdit->setEnabled(y);
    ui->messageView->setEnabled(y);
    ui->sendButton->setEnabled(y);
}

void MainWindow::sendMessage()
{
    QString message(ui->plainTextEdit->toPlainText());
    if (!message.isEmpty()) {
        _currentCommand.retranslateCommand(ChatCommand::Message,message);
        emit newCommand();
        ui->plainTextEdit->clear();
        // Manually add our own messages to messageView
        Chat::appendMessage(_currentCommand.participants().first(),message);
        ui->messageView->scrollToBottom();
    }
}

void MainWindow::scrollToBottom()
{
    ui->messageView->scrollToBottom();
}
/*
void MainWindow::readSettings()
{
    QSettings
}
*/
#ifdef AUDIO
void MainWindow::setupAudioToolbar(const QStringList & payloads)
{
    QMenu *audioStreamMenu = new QMenu("AudioStream", this);
    QAction *startAction = ui->actionStart_audiostream;
    startAction->setCheckable(true);
    startAction->setChecked(false);
    audioStreamMenu->setIcon(QIcon(":/icons/broadcast"));
    audioStreamMenu->addAction(startAction);
    audioStreamMenu->addSeparator();

    payloadsActionGroup = new QActionGroup(audioStreamMenu);
    foreach (QString payload, payloads) {
        QAction *payloadAction = new QAction(payload, payloadsActionGroup);
        audioStreamMenu->addAction(payloadAction);
        payloadAction->setEnabled(false);
        payloadAction->setCheckable(true);
    }
    payloadsActionGroup->setExclusive(true);
    payloadsActionGroup->actions().first()->setChecked(true);

    ui->mainToolBar->addAction(audioStreamMenu->menuAction());
    connect(startAction, SIGNAL(triggered(bool)), SIGNAL(startAudioStream(bool)));
    connect(startAction, SIGNAL(triggered(bool)), SLOT(enableAudioPayloads(bool)));
    connect(payloadsActionGroup, SIGNAL(triggered(QAction*)),SLOT(changeAudioPayload(QAction*)));
}

QString MainWindow::getAudioPayload()
{
    QAction * checkedPayload = payloadsActionGroup->checkedAction();
    if (!checkedPayload) {
        return "speex";
    }

    return checkedPayload->text();
}

void MainWindow::changeAudioPayload(QAction *action)
{
    emit audioStreamPayloadChanged(action->text());
}

void MainWindow::enableAudioPayloads(bool y)
{
    foreach (QAction * action, payloadsActionGroup->actions()) {
        action->setEnabled(y);
    }
}
#endif
