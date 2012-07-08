#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chat.h"
#include <QActionGroup>
#include <QAction>

namespace Ui {
    class MainWindow;
}

class MainWindow : public Chat {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

#ifdef AUDIO
    void setupAudioToolbar(const QStringList & payloads);
    QString getAudioPayload();
#endif
    void enableChat(bool);

signals:
#ifdef AUDIO
    void startAudioStream(bool);
    void audioStreamPayloadChanged(QString payload);
#endif
protected:
    void changeEvent(QEvent *e);

private slots:
#ifdef AUDIO
    void changeAudioPayload(QAction*);
    void enableAudioPayloads(bool);
#endif
    void sendMessage();
    void scrollToBottom();
private:
    Ui::MainWindow *ui;
#ifdef AUDIO
    QActionGroup *payloadsActionGroup;
#endif
};

#endif // MAINWINDOW_H
