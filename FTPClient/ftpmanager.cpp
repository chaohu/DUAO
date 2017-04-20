#include "ftpmanager.h"
#include <QDebug>

int getport(char *recvBuf);

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
    control_sock = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    FTPManager::host = host;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host.data());
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);


    //连接到服务器端命令端口
    connect(control_sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
    recv(control_sock,recvBuf,195,0);
    qDebug("链接命令端口返回消息:%s",recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    //登陆到服务器
    sprintf(sendBuf,"USER %s\r\n",username.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug("输入用户名:%s",recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    sprintf(sendBuf,"PASS %s\r\n",password.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug("输入密码返回消息:%s",recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    //切换服务器至被动态,连接数据端口
    sprintf(sendBuf,"PASV\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,50,0);
    qDebug("被动态返回信息:%s",recvBuf);
    int hehe = getport(recvBuf);
    qDebug("数据端口号："+hehe);
    addrSrv.sin_port = htons(hehe);
    memset(recvBuf,0,sizeof(recvBuf));
    data_sock = socket(AF_INET,SOCK_STREAM,0);
    connect(data_sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));

    //获取根目录列表
    sprintf(sendBuf,"LIST \r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(data_sock,recvBuf,195,0);
    qDebug("根目录信息:%s",recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    recv(data_sock,recvBuf,195,0);
    qDebug("根目录信息:%s",recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    return 1;
}


int FTPManager::setactvmode() {
    return 1;
}

int FTPManager::setpassmode() {
//    WSADATA wsaData;

    //发送让服务器切换至被动态的命令
    sprintf(sendBuf,"PASV\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,50,0);
    qDebug("被动态返回信息:%s",recvBuf);

//    /*初始化socket资源*/
//    if(WSAStartup(MAKEWORD(1,1),&wsaData) != 0) {
//        return 0;	//失败
//    }
//    if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
//        WSACleanup();
//        return 0;
//    }
//    data_sock = socket(AF_INET,SOCK_STREAM,0);
//    SOCKADDR_IN addrSrv;
//    addrSrv.sin_addr.S_un.S_addr = inet_addr(host.data());
//    addrSrv.sin_family = AF_INET;
//    addrSrv.sin_port = htons(port);


    //连接到服务器端数据端口
//    connect(data_sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
//    recv(data_sock,recvBuf,50,0);
//    qDebug(recvBuf);

    return 1;
}

int FTPManager::logoutserver(){
    closesocket(control_sock);
    WSACleanup();
    return 1;
}



//解析PASV命令服务器的返回数据得到数据端口号
int getport(char *recvBuf) {
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
