#include "downloadthread.h"

extern vector<unsigned> size_now_no;
extern QMutex mutex_process;

DownloadThread::DownloadThread(MainWindow *mainwindow,SOCKET data_sock,QString filename,unsigned size_all) {
    DownloadThread::mainwindow = mainwindow;
    DownloadThread::data_sock = data_sock;
    DownloadThread::filename = filename;
    DownloadThread::size_all = size_all;
    connect(this,SIGNAL(filedownload()),mainwindow,SLOT(flash_local_dir_list()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

DownloadThread::~DownloadThread() {

}

void DownloadThread::run() {
    FILE *file;
    qDebug()<<filename;
    if((file = fopen(filename.toLocal8Bit(),"wb")) == NULL) {
        qDebug("创建文件失败");
    }
    else {
        qDebug("创建文件成功");
        int size = 0,locate;
        mutex_process.lock();
        size_now_no.push_back(0);
        locate = size_now_no.size();
        mutex_process.unlock();
        char hehe[10];
        itoa(locate,hehe,10);
        qDebug(hehe);
        char recv_content[514];
        DownloadBar *downloadbar = new DownloadBar(mainwindow,size_all,locate-1);
        downloadbar->start();
        while((size = recv(data_sock,recv_content,512,0)) > 0) {
            fwrite(recv_content,1,size,file);
            mutex_process.lock();
            size_now_no[locate-1] += (unsigned)size;
            itoa(size_now_no[locate-1],hehe,10);
            mutex_process.unlock();
            memset(recv_content,0,sizeof(recv_content));
            qDebug(hehe);
        }
        fclose(file);
        closesocket(data_sock);
        emit filedownload();
    }
}


DownloadBar::DownloadBar(MainWindow *mainwindow,unsigned size_all,int locate) {
    DownloadBar::mainwindow = mainwindow;
    DownloadBar::size_all = size_all;
    DownloadBar::locate = locate;
    connect(this,SIGNAL(add_bar(int)),mainwindow,SLOT(add_progressbar(int)));
    connect(this,SIGNAL(flash_bar(int,uint)),mainwindow,SLOT(flash_bar(int,uint)));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

DownloadBar::~DownloadBar() {

}

void DownloadBar::run() {
    unsigned value = 0;
    emit add_bar(locate);
    qDebug("bar");
    char hehe[10];
    while(value < 100) {
        qDebug("late111");
        mutex_process.lock();
        value = (size_now_no[locate]*100)/size_all;
        mutex_process.unlock();
        emit flash_bar(locate,value);
        msleep(500);
        itoa(value,hehe,10);
        qDebug(hehe);
    }
    qDebug("late222");
}
