#include <QtNetwork>
#include "manager.h"

Manager::Manager(QObject *parent) :
    QTcpServer(parent),
    connectionMapper(new QSignalMapper(this)),
    chatMapper(new QSignalMapper(this)),
    audioStreamAvailable(false),
    _settings(new QSettings("Shadowrun Comlink","MetaLink Server",this))
{
    readSettings();
    listen(QHostAddress::Any, _listenPort);

    connect(connectionMapper, SIGNAL(mapped(int)), this, SLOT(parseIncomingChatCommand(int)));
    connect(chatMapper, SIGNAL(mapped(int)), this, SLOT(parseOutcomingChatCommand(int)));

    MainWindow *main = new MainWindow();
    chats.append(main);
    connect(main, SIGNAL(newCommand()), chatMapper, SLOT(map()));
    chatMapper->setMapping(main, main->id());
#ifdef AUDIO
    connect(main, SIGNAL(startAudioStream(bool)), SLOT(startAudioStream(bool)));
    connect(main, SIGNAL(audioStreamPayloadChanged(QString)), SLOT(updateAudioStream(QString)));
#endif

    tabWidget.setTabsClosable(true);
    tabMain.setCentralWidget(&tabWidget);
    connect(&tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(destroyTab(int)));

    main->show();
}

void Manager::readSettings()
{
    if (_settings->contains("listen port")) {
        _listenPort = _settings->value("listen port").toInt();
    } else {
        _listenPort = 1337;
    }
#ifdef AUDIO
/* TODO: Save payload in settings
if (_settings->contains("Audio payload")) {
        MainWindow * main = (MainWindow*)chats.at(0);
        main->changeAudioPayload(_settings->value("Audio payload").toString());
    } else {
        _settings->setValue("Audio payload","none");
        MainWindow * main = (MainWindow*)chats.at(0);
        main->changeAudioPayload("none");
    }
*/
    if (_settings->contains("Audio enable")) {
        startAudioStream(_settings->value("Audio enable").toBool());
    }
#endif
}

void Manager::sendCommandToMany(ChatCommand &command, const QStringList &recipients)
{
    foreach (Connection *connection, connections) {
        if (recipients.contains(connection->nick()))
            connection->send(Connection::Chat, command.commandString());
    }
}

void Manager::sendCommandToAll(ChatCommand &command)
{
    foreach (Connection *connection, connections) {
        connection->send(Connection::Chat, command.commandString());
    }
}

Chat * Manager::initNewChat(QString by)
{
    // Create new chat from Nick
    Chat *chat = new ChatWindow(by, chats.first());
    // Connect the commandMapper
    connect(chat, SIGNAL(newCommand()),chatMapper,SLOT(map()));
    chatMapper->setMapping(chat, chat->id());
    // Add chat to list
    chats.append(chat);
    // Add chat to tabs
    tabWidget.addTab(chat,chat->command().participants().join(", "));
    connect(&tabMain,SIGNAL(closed()),chat,SLOT(close()));

    tabMain.show();
    return chat;
}

void Manager::destroyTab(int index)
{
    Chat * chat = (Chat*)tabWidget.widget(index);
    if (chat != 0) {
        chat->close();
        tabWidget.removeTab(index);
        if (tabWidget.count() < 1) {
            tabMain.hide();
        }
    }
}

Chat* Manager::findChat(unsigned int chatID)
{
    foreach (Chat *chat, chats) {
        if (*chat == chatID) {
            return chat;
        }
    }
    return 0;
}

void Manager::updateConnections()
{
    foreach (ServerConnection *connection, connections) {
        if (connection->state() < QAbstractSocket::ConnectedState) {
            // Update participants
            foreach (Chat *chat, chats) {
                chat->removeParticipant(connection->nick());
                ChatCommand command(chat->id(), ChatCommand::List, chat->participants());
                sendCommandToMany(command, chat->participants());;
            }
            connections.removeOne(connection);
            connection->deleteLater();
        }
    }
    if (connections.empty()) {
        ((MainWindow*)chats.first())->enableChat(false);
    }
}

void Manager::updateNicks()
{
    QStringList connectedNicks;
    foreach (ServerConnection *connection, connections) {
        connectedNicks << connection->nick();
    }
    Chat * main = chats.first();
    main->setParticipants(connectedNicks);
    ChatCommand command = main->command();
    command.retranslateCommand(ChatCommand::List, connectedNicks);
    sendCommandToAll(command);
}

void Manager::parseIncomingChatCommand(int connectionID)
{
    foreach (ServerConnection *connection, connections) {
        if (connectionID == connection->id()) {
            ChatCommand command = connection->currentCommand();
            Chat * chat = findChat(command.chatID());
            QStringList recipients;
            switch (command.type()) {
            case ChatCommand::Init:
                // Start new Chat
                chat = initNewChat(connection->nick());
                if (chat) {
                    recipients = command.participants();
                    // Invite ALL participants
                    command.retranslateCommand(ChatCommand::Invite,
                                               chat->participants(),
                                               chat->id());
                    sendCommandToMany(command,recipients);
                }
                break;
            case ChatCommand::Invite:
                // Invite new participants
                recipients = command.participants();
                command.retranslateCommand(ChatCommand::Invite, chat->participants());
                sendCommandToMany(command,recipients);
                break;
            case ChatCommand::Accept:
                chat->addParticipant(connection->nick());
                command.retranslateCommand(ChatCommand::List,chat->participants());
                sendCommandToMany(command,chat->participants());
                break;
            case ChatCommand::Reject:
                // What to do, what to do?
                break;
            case ChatCommand::Leave:
                chat->removeParticipant(connection->nick());
                command.retranslateCommand(ChatCommand::List,chat->participants());
                sendCommandToMany(command,chat->participants());
                break;
            case ChatCommand::Message:
                chat->appendMessage(command.participants().first(), command.message());
                sendCommandToMany(command,chat->participants());
                break;
            default:
                break;
            }
        }
    }
}

void Manager::parseOutcomingChatCommand(int chatID)
{
    Chat * chat = findChat(chatID);
    if (chat) {
        qDebug() << "Parsing outgoing ChatCommand from Chat " << QString::number(chatID);
        ChatCommand command = chat->command();
        switch (command.type()) {
        case ChatCommand::Init:
            break;
        case ChatCommand::Invite:
            break;
        case ChatCommand::Accept:
            break;
        case ChatCommand::Reject:
            // What to do, what to do?
            break;
        case ChatCommand::Leave:
            sendCommandToMany(command, command.participants());
            chat->removeParticipant(command.participants());
            break;
        case ChatCommand::Message:
            sendCommandToMany(command, chat->participants());
            break;
        default:
            break;
        }
    }

}

void Manager::incomingConnection(int socketDescriptor)
{
    ServerConnection *connection = new ServerConnection(this);
    if(!connection->setSocketDescriptor(socketDescriptor))
        qDebug() << "Failed to set socket descriptor";

    connections.append(connection);

    connect(connection, SIGNAL(disconnected()), this, SLOT(updateConnections()));
    connect(connection, SIGNAL(nickReceived()), this, SLOT(updateNicks()));
    connect(connection, SIGNAL(newChatCommand()), connectionMapper, SLOT(map()));
    connectionMapper->setMapping(connection, connection->id());

    MainWindow *main = (MainWindow*)chats.first();
    main->enableChat(true);
#ifdef AUDIO
    if (audioStreamAvailable) {
        QString payload = main->getAudioPayload();
        connection->startAudioStream(true, payload);
        connection->send(Connection::Audio, payload);
    }
#endif
}

#ifdef AUDIO
void Manager::startAudioStream(bool start)
{
    foreach (ServerConnection * connection, connections) {
        if (start) {
            MainWindow * main = (MainWindow*)chats.first();
            QString payload = main->getAudioPayload();
            qDebug() << "Starting audioStream";
            connection->startAudioStream(start, payload);
            connection->send(Connection::Audio, payload);
        } else {
            connection->startAudioStream(start, QString());
            connection->send(Connection::Audio, "none");
        }
    }
    audioStreamAvailable = start;
}

void Manager::updateAudioStream(QString payload)
{
    foreach (ServerConnection *connection, connections) {
        connection->changeAudioStreamPayload(payload);
        connection->send(Connection::Audio, payload);
    }
}
#endif
