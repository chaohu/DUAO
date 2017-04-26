#ifndef NEWTHREAD_H
#define NEWTHREAD_H
#include "mainwindow.h"
#include <QThread>
#include <winsock2.h>
#include <QDebug>

class NewThread : public QThread
{
    Q_OBJECT

signals:
    void filedownload();

private:
    MainWindow *mainwindow;
    SOCKET data_sock;
    QString filename;

public:
    NewThread(SOCKET data_sock,QString filename);
    ~NewThread();

    void run();

};


#endif // NEWTHREAD_H
