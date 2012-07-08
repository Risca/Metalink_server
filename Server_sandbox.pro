# -------------------------------------------------
# Project created by QtCreator 2010-05-13T16:30:14
# -------------------------------------------------

#DEFINES += AUDIO
QT += network
TARGET = Server_sandbox
TEMPLATE = app
INCLUDEPATH += ../Common/ChatCommand \
    ../Common/Chat \
    ../Common/Connection \
    ../Common/customPlainTextEdit
LIBS += -L../Common-libs -lChatCommand \
    -lChat \
    -lConnection \
    -lcustomplaintexteditplugin
SOURCES += main.cpp \
    manager.cpp \
    chatwindow.cpp \
    mainwindow.cpp \
    serverconnection.cpp \
    chattabs.cpp
HEADERS += manager.h \
    chatwindow.h \
    mainwindow.h \
    serverconnection.h \
    chattabs.h
FORMS += mainwindow.ui \
    chatwindow.ui
RESOURCES += Icons.qrc

# Custom audio stuff here
contains(DEFINES, AUDIO) {
    message(Building with audio enabled...)
    HEADERS += audiostream.h
    SOURCES += audiostream.cpp
    LIBS += -lmediastreamer
}
