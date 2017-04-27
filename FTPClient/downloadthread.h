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
    void filedownload();

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


class DownloadBar : public QThread
{
    Q_OBJECT

signals:
    void add_bar(int num);
    void flash_bar(int num,unsigned value);

private:
    MainWindow *mainwindow;
    QProgressBar *download_bar;
    unsigned size_all;
    int locate;

public:
    DownloadBar(MainWindow *mainwindow,unsigned size_all,int locate);
    ~DownloadBar();

    void run();

};

#endif // DOWNLOADTHREAD_H
