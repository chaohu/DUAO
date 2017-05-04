#include "uploadthread.h"
#include "duprogressbar.h"
#include <QSemaphore>

extern QList<unsigned> size_now_no;
extern QMutex mutex_process;

extern QSemaphore bar_list_queen;

UploadThread::UploadThread(MainWindow *mainwindow, SOCKET data_sock, QString filename, long long offset) {
    UploadThread::mainwindow = mainwindow;
    UploadThread::data_sock = data_sock;
    UploadThread::filename = filename;
    UploadThread::offset = offset;
    connect(this,SIGNAL(fileupload()),mainwindow,SLOT(f_ch_server_dir()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

UploadThread::~UploadThread() {

}

void UploadThread::run() {
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug("读取文件失败");
    }
    else {
        int aaa;
        char hehe[10];
        qDebug("读取文件成功");
        file.seek(offset);
        bar_list_queen.acquire();
        char send_content[514];
        int size = 0,locate;
        mutex_process.lock();
        size_now_no.push_back(0);
        locate = size_now_no.size();
        mutex_process.unlock();
        DUProgressBar *duprogressbar = new DUProgressBar(mainwindow,file.size(),locate-1);
        duprogressbar->start();
        while((size = file.read(send_content,512)) > 0) {
            aaa = send(data_sock,send_content,size,0);
            itoa(aaa,hehe,10);
            qDebug(hehe);
            mutex_process.lock();
            size_now_no[locate-1] += (unsigned)size;
            mutex_process.unlock();
            memset(send_content,0,sizeof(send_content));
        }
        file.close();
        closesocket(data_sock);
        emit fileupload();
        bar_list_queen.release();
    }
}
