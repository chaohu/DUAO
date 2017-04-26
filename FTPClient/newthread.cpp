#include "newthread.h"

NewThread::NewThread(SOCKET data_sock,QString filename) {
    NewThread::data_sock = data_sock;
    NewThread::filename = filename;
    connect(this,SIGNAL(filedownload()),mainwindow,SLOT(flash_server_dir_list()));
}

NewThread::~NewThread() {

}

void NewThread::run() {
    FILE *file;
    qDebug()<<filename;
    if((file = fopen(filename.toLocal8Bit(),"wb")) == NULL) {
        qDebug("创建文件失败");
    }
    else {
        qDebug("创建文件成功");
        char recv_content[514];
        int size = 0;
        while((size = recv(data_sock,recv_content,512,0)) > 0) {
            fwrite(recv_content,1,size,file);
            memset(recv_content,0,sizeof(recv_content));
        }
        fclose(file);
        closesocket(data_sock);
        emit filedownload();
    }
}
