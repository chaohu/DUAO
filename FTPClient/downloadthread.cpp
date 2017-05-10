#include "downloadthread.h"
#include "duprogressbar.h"
#include "ftpmanager.h"
#include <QSemaphore>
#include <QFile>

extern QList<unsigned> size_now_no;
extern QMutex mutex_process;

extern QSemaphore bar_list_queen;

extern FTPManager *ftpmanager;

DownloadThread::DownloadThread(MainWindow *mainwindow,SOCKET data_sock,QString filename,unsigned size_all) {
    DownloadThread::mainwindow = mainwindow;
    DownloadThread::data_sock = data_sock;
    DownloadThread::filename = filename;
    DownloadThread::size_all = size_all;
    connect(this,SIGNAL(check_file(bool)),mainwindow,SLOT(check_file(bool)));
    connect(this,SIGNAL(filedownload(QString)),mainwindow,SLOT(file_downloaded(QString)));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

DownloadThread::~DownloadThread() {

}

void DownloadThread::run() {
    QFile file(filename.toLocal8Bit().data());
    qDebug()<<filename;
    if(!file.open(QIODevice::WriteOnly)) {
        qDebug("创建文件失败");
    }
    else {
        qDebug("创建文件成功");
        file.seek(file.size());
        bar_list_queen.acquire();
        int size = 0,locate;
        mutex_process.lock();
        size_now_no.append(0);
        locate = size_now_no.size();
        mutex_process.unlock();
        char hehe[10];
        itoa(locate,hehe,10);
        qDebug(hehe);
        char recv_content[514];
        DUProgressBar *duprogressbar = new DUProgressBar(mainwindow,filename,size_all,locate-1);
        duprogressbar->start();
        while((size = recv(data_sock,recv_content,512,0)) > 0) {
            file.write(recv_content,size);
            mutex_process.lock();
            size_now_no[locate-1] += (unsigned)size;
            itoa(size_now_no[locate-1],hehe,10);
            mutex_process.unlock();
            memset(recv_content,0,sizeof(recv_content));
            qDebug(hehe);
        }
//        mutex_process.lock();
//        size_now =  size_now_no[locate-1];
//        mutex_process.unlock();
        file.close();
        closesocket(data_sock);

//        recv(ftpmanager->getcontrolsock(),recv_content,195,0);
//        mainwindow->add_log(recv_content);
//        memset(recv_content,0,sizeof(recv_content));

        if(file.size() == size_all) emit check_file(true);
        else emit check_file(false);
        emit filedownload(filename);
        bar_list_queen.release();
    }
}
