#include "ftpmanager.h"
#include <QDebug>

FTPManager::FTPManager() {

}

FTPManager::~FTPManager() {

}

int FTPManager::loginserver(const std::string host,const std::string username,const std::string password,const int port) {
    WSADATA wsaData;
    /*初始化socket资源*/
    if(WSAStartup(MAKEWORD(1,1),&wsaData) != 0) {
        return 0;	//失败
    }
    if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        return 0;
    }

    //初始化socket
    sockClient = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host.data());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);

    //连接到服务器端
    connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
    char recvBuf[50];
    recv(sockClient,recvBuf,50,0);
    qDebug(recvBuf);

    //登陆到服务器
    char sendBuf[20];
    sprintf(sendBuf,"USER %s\r\n",username.data());
    qDebug(sendBuf);
    send(sockClient,sendBuf,strlen(sendBuf),0);
    recv(sockClient,recvBuf,50,0);
    qDebug(recvBuf);
    sprintf(sendBuf,"PASS %s\r\n",password.data());
    qDebug(sendBuf);
    send(sockClient,sendBuf,strlen(sendBuf),0);
    recv(sockClient,recvBuf,50,0);
    qDebug(recvBuf);

    return 1;
}

int FTPManager::logoutserver(){
    closesocket(sockClient);
    WSACleanup();
    return 1;
}
