#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H
#include "mainwindow.h"
#include <QThread>
#include <winsock2.h>
#include <QDebug>
#include <QMutex>
#include <QProgressBar>


class DownloadThread : public QThread
{
    Q_OBJECT

signals:
    void check_file(bool check);
    void filedownload(QString filename);

private:
    MainWindow *mainwindow;
    SOCKET data_sock;
    QString filename;
    unsigned size_all;

public:
    DownloadThread(MainWindow *mainwindow,SOCKET data_sock,QString filename,unsigned size_all);
    ~DownloadThread();

    void run();

};

#endif // DOWNLOADTHREAD_H
