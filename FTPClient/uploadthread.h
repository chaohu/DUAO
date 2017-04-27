#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H
#include "mainwindow.h"
#include <QThread>
#include <winsock2.h>
#include <QDebug>

class UploadThread : public QThread
{
    Q_OBJECT

signals:
    void fileupload();

private:
    MainWindow *mainwindow;
    SOCKET data_sock;
    QString filename;

public:
    UploadThread(MainWindow *mainwindow,SOCKET data_sock,QString filename);
    ~UploadThread();

    void run();

};


#endif // UPLOADTHREAD_H
