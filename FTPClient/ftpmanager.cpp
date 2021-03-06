#include "ftpmanager.h"
#include "downloadthread.h"
#include "uploadthread.h"
#include "heartconn.h"
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QHostAddress>
#include <QDebug>
#include <QSemaphore>

QList<unsigned> size_now_no;
QMutex mutex_process;

QSemaphore bar_list_queen(3);
HeartConn *heartconn;

FTPManager::FTPManager(MainWindow *mainwindow) {
    FTPManager::mainwindow = mainwindow;
    QHostInfo local_host_info = QHostInfo::fromName(QHostInfo::localHostName());
    foreach (QHostAddress local_ip, local_host_info.addresses()) {
        if(local_ip.protocol() == QAbstractSocket::IPv4Protocol) {
            qDebug()<<local_ip.toString();
            sscanf(local_ip.toString().toLatin1().data(),"%d.%d.%d.%d",&local_ip_part1,&local_ip_part2,&local_ip_part3,&local_ip_part4);
            break;
        }
    }
}

FTPManager::~FTPManager() {

}


int FTPManager::loginserver(const std::string host,const std::string username,const std::string password,const int port) {
    FTPManager::host = host;

    if(init_sock() == 0) return 0;

    if(!socket_conn(&control_sock,port)) return 0;

    recv(control_sock,recvBuf,195,0);
    qDebug("链接命令端口返回消息:%s",recvBuf);
    mainwindow->add_log(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    //登陆到服务器
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"USER %s\r\n",username.data());
    if(send_order(sendBuf) != 331) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"PASS %s\r\n",password.data());
    if(send_order(sendBuf) != 230) return 0;

    return 1;
}

int FTPManager::init_sock() {

    //初始化命令、数据socket
    WSADATA wsaData;

    /*初始化socket资源*/
    if(WSAStartup(MAKEWORD(1,1),&wsaData) != 0) {
        return 0;	//失败
    }
    if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        return 0;	//失败
    }
    control_sock = socket(AF_INET,SOCK_STREAM,0);

    return 1;
}

int FTPManager::send_order(string order) {
    char recv_message[200];
    int state_code = -1;
    send(control_sock,order.data(),strlen(order.data()),0);
    recv(control_sock,recv_message,195,0);
    qDebug("%s",recv_message);
    mainwindow->add_log(recvBuf);
    sscanf(recv_message,"%d ",&state_code);
    return state_code;
}

int FTPManager::socket_conn(SOCKET *_socket,const int port) {
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host.data());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    qDebug(host.data());
    char hehe[10];
    itoa(port,hehe,10);
    qDebug(hehe);


    //连接到服务器端端口
    if(SOCKET_ERROR == connect(*_socket,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR))) {
        char error_info[10];
        itoa(WSAGetLastError(),error_info,10);
        qDebug(error_info);
        return 0;
    }
    else return 1;
}

int FTPManager::socket_accept(SOCKET *_socket) {
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host.data());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(20);
    int len = sizeof(SOCKADDR);
    qDebug("conn");
//    *_socket = socket(AF_INET,SOCK_STREAM,0);
    *_socket = accept(data_sock,(SOCKADDR*)&addrSrv,&len);
    if(INVALID_SOCKET == *_socket) {
        char error_info[10];
        itoa(WSAGetLastError(),error_info,10);
        qDebug(error_info);
        return 0;
    }
    else return 1;
}

//随机生成的端口范围：1025-65531
int FTPManager::setactvmode() {
    data_sock = socket(AF_INET,SOCK_STREAM,0);
    int port_part1 = 0;
    int port_part2 = 0;
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    do {
        char error_info[10];
        itoa(WSAGetLastError(),error_info,10);
        qDebug(error_info);
        port_part1 = rand()%251 + 4;
        port_part2 = rand()%250 + 1;
        addrSrv.sin_port = htons(port_part1 * 256 + port_part2);
    } while(bind(data_sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR)) == SOCKET_ERROR);
    act_port_part1 = port_part1;
    act_port_part2 = port_part2;

    listen(data_sock,5);

    return 1;
}

int FTPManager::setpassmode() {
    int state_code = -1;
    //发送让服务器切换至被动态的命令
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"PASV\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug("被动态返回信息:%s",recvBuf);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    if(state_code != 227) {
        memset(recvBuf,0,sizeof(recvBuf));
        return 0;
    }
    int port = getport(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    data_sock = socket(AF_INET,SOCK_STREAM,0);
    return socket_conn(&data_sock,port);
}

//获取服务器当前工作目录
int FTPManager::get_server_current_path() {
    int state_code = -1;
    int i = 0,j = 0,count = 0;
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"PWD \r\n");
    send(control_sock,sendBuf,sizeof(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    if(state_code != 257) {
        memset(recvBuf,0,sizeof(recvBuf));
        return 0;
    }
    memset(server_current_path,0,sizeof(server_current_path));
    while(count < 2) {
        if(recvBuf[i] == '"') {
            count++;
            i++;
        }
        if(count == 1) {
            server_current_path[j] = recvBuf[i];
            j++;
        }
        i++;
    }
    memset(recvBuf,0,sizeof(recvBuf));
    return 1;
}

//改变服务器工作目录
int FTPManager::ch_server_dir(string path) {
    qDebug(path.data());
//    if(path == "..") {
//        if(send_order("CDUP\r\n") == 200) return 1;
//        else return 0;
//    }
//    else {
        char order[270];
        sprintf(order,"CWD %s\r\n",path.data());
        if(send_order(order) == 250) return 1;
        else return 0;
//    }
}

//获取服务器目录文件列表
int FTPManager::get_dir_list() {
    SOCKET _socket;
    int state_code = -1;

    if(mode_flag) {
        if(!setactvmode()) return 0;
        memset(sendBuf,0,sizeof(sendBuf));
        sprintf(sendBuf,"PORT %d,%d,%d,%d,%d,%d\r\n",127,0,0,1,act_port_part1,act_port_part2);
        if(send_order(sendBuf) != 200) return 0;
    }
    else {
        if(!setpassmode()) return 0;
        _socket = data_sock;
    }

    //获取根目录列表
    if(send_order("LIST \r\n") != 150) return 0;
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 226) return 0;
    server_dir_list_info.clear();
    qDebug("LIST返回信息：");

    if(mode_flag) if(!socket_accept(&_socket)) return 0;

    while(recv(_socket,recvBuf,195,0) > 0) {
        server_dir_list_info.append(recvBuf);
        memset(recvBuf,0,sizeof(recvBuf));
    }
    memset(recvBuf,0,sizeof(recvBuf));
    qDebug("%s",server_dir_list_info.data());

    closesocket(_socket);
    if(mode_flag) closesocket(data_sock);
    return 1;
}


//主动模式下载
int FTPManager::file_download_act(QString filename) {
    QFile file(filename);
    int state_code;
    unsigned size_all;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"SIZE %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d %ud",&state_code,&size_all);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) return 0;

    if(file.size() == size_all) return 1;

    sprintf(sendBuf,"REST %lld\r\n",file.size());
    send_order(sendBuf);

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"TYPE I\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    if(!setactvmode()) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"PORT %d,%d,%d,%d,%d,%d\r\n",127,0,0,1,act_port_part1,act_port_part2);
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"RETR %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 150) return 0;

    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 226) return 0;

    SOCKET file_socket = socket(AF_INET,SOCK_STREAM,0);
//    SOCKET file_socket;
    if(socket_accept(&file_socket)) {
        qDebug("wolai");
//        flag = writetofile(file_socket,filename.data());
//        closesocket(file_socket);
        DownloadThread *downloadthread = new DownloadThread(mainwindow,file_socket,filename,size_all);
        downloadthread->start();
    }
    closesocket(data_sock);

    return 1;
}

//被动模式从服务器下载
int FTPManager::file_download_pas(QString filename) {
    QFile file(filename);
    char hehe[10];
    itoa(file.size(),hehe,10);
    qDebug(hehe);
    int state_code;
    unsigned size_all;

    qDebug() << filename;
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"SIZE %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d %ud",&state_code,&size_all);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) return 0;

    if(file.size() == size_all) return 1;

    sprintf(sendBuf,"REST %lld\r\n",file.size());
    send_order(sendBuf);

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"TYPE I\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    if(!setpassmode()) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"RETR %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 150) return 0;

    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

//    flag = writetofile(data_sock,filename.data());
//    closesocket(data_sock);
    DownloadThread *downloadthread = new DownloadThread(mainwindow,data_sock,filename,size_all);
    downloadthread->start();


    return 1;
}

//主动模式上传文件至服务器
int FTPManager::file_upload_act(QString filename) {
    QFile file(filename);
    int state_code = -1;
    long long offset = 0;
    int size_all;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"SIZE %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d %ud",&state_code,&size_all);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) offset = 0;
    else if(size_all == file.size()) return 1;
    else offset = (long long)size_all;

    if(send_order("TYPE I\r\n") != 200) return 0;

    if(!setactvmode()) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"PORT %d,%d,%d,%d,%d,%d\r\n",127,0,0,1,act_port_part1,act_port_part2);
    if(send_order(sendBuf) != 200) return 0;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"APPE %s\r\n",filename.toLocal8Bit().data());
    if(send_order(sendBuf) != 150) return 0;

    SOCKET file_sock = socket(AF_INET,SOCK_STREAM,0);
    if(!socket_accept(&file_sock)) return 0;

//    flag = readfromfile(file_socket,filename.data());
//    closesocket(file_socket);

    UploadThread *uploadtheread = new UploadThread(mainwindow,file_sock,filename,offset);
    uploadtheread->start();

    closesocket(data_sock);

//    recv(control_sock,recvBuf,195,0);
//    mainwindow->add_log(recvBuf);
//    sscanf(recvBuf,"%d ",&state_code);
//    qDebug(recvBuf);
//    memset(recvBuf,0,sizeof(recvBuf));
//    if(state_code != 226) return 0;
    return 1;
}


//被动模式上传文件至服务器
int FTPManager::file_upload_pas(QString filename) {
    QFile file(filename);
    int state_code = -1;
    long long offset = 0;
    int size_all;

    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"SIZE %s\r\n",filename.toLocal8Bit().data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    mainwindow->add_log(recvBuf);
    sscanf(recvBuf,"%d %ud",&state_code,&size_all);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) offset = 0;
    else if(size_all == file.size()) return 1;
    else offset = (long long)size_all;


    if(send_order("TYPE I\r\n") != 200) return 0;
    if(!setpassmode()) return 0;
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"APPE %s\r\n",filename.toLocal8Bit().data());
    if(send_order(sendBuf) != 150) return 0;

    UploadThread *uploadthread = new UploadThread(mainwindow,data_sock,filename,offset);
    uploadthread->start();

    return 1;
}


//客户端读取文件
int FTPManager::readfromfile(SOCKET _socket, const char *filename) {
    FILE *file;
    QString filename_gbk(filename);
    if((file = fopen(filename_gbk.toLocal8Bit(),"rb")) == NULL) {
        qDebug("读取文件失败");
        return 0;
    }
    else {
        qDebug("读取文件成功");
        char send_content[514];
        int size = 0;
        while((size = fread(send_content,1,512,file)) > 0) {
            send(_socket,send_content,size,0);
            memset(send_content,0,sizeof(send_content));
        }
        fclose(file);
        return 1;
    }
}


int FTPManager::logoutserver(){
    memset(sendBuf,0,sizeof(sendBuf));
    sprintf(sendBuf,"QUIT \r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    closesocket(control_sock);
    closesocket(data_sock);
    WSACleanup();
    return 1;
}



//解析PASV命令服务器的返回数据得到数据端口号
int FTPManager::getport(const char *recvBuf) {
    int i = 0,j = 0,k = 0;
    int count = 0;
    char part1[10];
    char part2[10];
    while(recvBuf[i] != ')') {
        if(recvBuf[i] == ',') {
            count++;
        }
        else if(count == 4) {
            part1[j] = recvBuf[i];
            j++;
        }
        else if(count == 5) {
            part2[k] = recvBuf[i];
            k++;
        }
        i++;
    }
    part1[j] = '\0';
    part2[k] = '\0';
    return atoi(part1)*256+atoi(part2);
}

//获取当前模式
int FTPManager::getmode() {
    return mode_flag;
}

//设置当前模式
int FTPManager::setmode(int mode_flag) {
    FTPManager::mode_flag = mode_flag;
    return 1;
}

SOCKET FTPManager::getdatasock() {
    mode_flag = 0;
    return data_sock;
}

SOCKET FTPManager::getcontrolsock() {
    return control_sock;
}

//断点续传下载被动
int FTPManager::cont_download(QString filename, int len) {
    return 1;
}

//断点续传上传
int FTPManager::cont_upload(QString filename, int len) {
    return 1;
}
