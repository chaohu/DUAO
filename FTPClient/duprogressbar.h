#ifndef DUPROGRESSBAR_H
#define DUPROGRESSBAR_H

#include "mainwindow.h"
#include <QThread>
#include <winsock2.h>
#include <QDebug>
#include <QMutex>
#include <QProgressBar>


class DUProgressBar: public QThread
{
    Q_OBJECT

signals:
    void add_bar(int num,QString filename);
    void flash_bar(int num,unsigned value);

private:
    MainWindow *mainwindow;
    QString filename;
    unsigned size_all;
    int locate;

public:
    DUProgressBar(MainWindow *mainwindow,QString filename,unsigned size_all,int locate);
    ~DUProgressBar();

    void run();

};

#endif // DUPROGRESSBAR_H
