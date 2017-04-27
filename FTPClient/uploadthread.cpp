#include "uploadthread.h"

UploadThread::UploadThread(MainWindow *mainwindow, SOCKET data_sock, QString filename) {
    UploadThread::mainwindow = mainwindow;
    UploadThread::data_sock = data_sock;
    UploadThread::filename = filename;
    connect(this,SIGNAL(fileupload()),mainwindow,SLOT(f_ch_server_dir()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

UploadThread::~UploadThread() {

}

void UploadThread::run() {
    FILE *file;
    if((file = fopen(filename.toLocal8Bit(),"rb")) == NULL) {
        qDebug("读取文件失败");
    }
    else {
        qDebug("读取文件成功");
        char send_content[514];
        int size = 0;
        while((size = fread(send_content,1,512,file)) > 0) {
            send(data_sock,send_content,size,0);
            memset(send_content,0,sizeof(send_content));
        }
        fclose(file);
        closesocket(data_sock);
        emit fileupload();
    }
}
