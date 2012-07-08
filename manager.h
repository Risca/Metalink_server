#ifndef MANAGER_H
#define MANAGER_H

#include <QTcpServer>
#include <QList>
#include <QSignalMapper>
#include <QSettings>
#include "serverconnection.h"
#include "chat.h"
#include "mainwindow.h"
#include "chatwindow.h"
#include "chattabs.h"

class Manager : public QTcpServer
{
Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);

protected:
    void incomingConnection(int socketDescriptor);

private:
    QList<ServerConnection*> connections;
    QList<Chat*> chats;
    QSignalMapper *connectionMapper;
    QSignalMapper *chatMapper;
    bool audioStreamAvailable;
    ChatTabs tabMain;
    QTabWidget tabWidget;
    QSettings * _settings;
    int _listenPort;

    void readSettings();
    void sendCommandToMany(ChatCommand &command, const QStringList &recipients);
    void sendCommandToAll(ChatCommand &command);
    Chat * initNewChat(QString);
    Chat * findChat(unsigned int chatID);

private slots:
    void updateConnections();
    void updateNicks();
    void parseIncomingChatCommand(int connectionID);
    void parseOutcomingChatCommand(int chatID);
    void destroyTab(int);
#ifdef AUDIO
    void startAudioStream(bool);
    void updateAudioStream(QString payload);
#endif
};

#endif // MANAGER_H
