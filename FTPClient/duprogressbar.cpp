#include "duprogressbar.h"

extern QList<unsigned> size_now_no;
extern QMutex mutex_process;

DUProgressBar::DUProgressBar(MainWindow *mainwindow,unsigned size_all,int locate) {
    DUProgressBar::mainwindow = mainwindow;
    DUProgressBar::size_all = size_all;
    DUProgressBar::locate = locate;
    connect(this,SIGNAL(add_bar(int)),mainwindow,SLOT(add_progressbar(int)));
    connect(this,SIGNAL(flash_bar(int,uint)),mainwindow,SLOT(flash_bar(int,uint)));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

DUProgressBar::~DUProgressBar() {

}

void DUProgressBar::run() {
    unsigned value = 0;
    emit add_bar(locate);
    qDebug("bar");
    char hehe[10];
    while(value < 100) {
        qDebug("late111");
        if(size_all<=0) value = 100;
        else {
            mutex_process.lock();
            value = (size_now_no[locate]*100)/size_all;
            mutex_process.unlock();
        }
        emit flash_bar(locate,value);
        msleep(500);
        itoa(value,hehe,10);
        qDebug(hehe);
    }
    qDebug("late222");
}
