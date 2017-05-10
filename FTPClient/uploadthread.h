#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H
#include "mainwindow.h"
#include <QThread>
#include <winsock2.h>
#include <QDebug>
#include <QFile>

class UploadThread : public QThread
{
    Q_OBJECT

signals:
    void fileupload(QString filename);

private:
    MainWindow *mainwindow;
    SOCKET data_sock;
    QString filename;
    long long offset;

public:
    UploadThread(MainWindow *mainwindow,SOCKET data_sock,QString filename,long long offset);
    ~UploadThread();

    void run();

};


#endif // UPLOADTHREAD_H
