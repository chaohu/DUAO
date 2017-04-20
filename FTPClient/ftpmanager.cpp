#include "ftpmanager.h"
#include <QDebug>

FTPManager::FTPManager() {

}

FTPManager::~FTPManager() {

}

result_login FTPManager::loginserver(const std::string host,const std::string username,const std::string password,const int port) {
    FTPManager::host = host;
    result_login result;

    result_socket_conn conn_result = socket_conn(host.data(),port);
    if(conn_result.state == 1) {
        control_sock = conn_result._socket;

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
        setpassmode();

        //获取根目录列表
        sprintf(sendBuf,"LIST \r\n");
        send(control_sock,sendBuf,strlen(sendBuf),0);
        qDebug("LIST返回信息：");
        string dir_info = "";
        while(recv(data_sock,recvBuf,195,0) > 0) {
            dir_info.append(recvBuf);
            memset(recvBuf,0,sizeof(recvBuf));
        }
        memset(recvBuf,0,sizeof(recvBuf));
        qDebug("%s",dir_info.data());
        result.dir_info = dir_info;
    }
    result.state = conn_result.state;
    return result;
}

result_socket_conn FTPManager::socket_conn(const char *host,const int port) {
    result_socket_conn result;
    WSADATA wsaData;
    SOCKET _socket;
    /*初始化socket资源*/
    if(WSAStartup(MAKEWORD(1,1),&wsaData) != 0) {
        result.state = 0;
        return result;	//失败
    }
    if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        result.state = 0;
        return result;	//失败
    }

    //初始化socket
    _socket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);


    //连接到服务器端命令端口
    if(SOCKET_ERROR == connect(_socket,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR))) {
        if(errno == 0) {
            qDebug("socket已经连接");
            result.state = 2;
            return result;
        }
        else {
            qDebug("socket已经连接");
            result.state = 0;
            return result;
        }
    }
    else {
        result.state = 1;//正常连接
        result._socket = _socket;
        return result;
    }
}


int FTPManager::setactvmode() {
    return 1;
}

int FTPManager::setpassmode() {
    //发送让服务器切换至被动态的命令
    sprintf(sendBuf,"PASV\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug("被动态返回信息:%s",recvBuf);
    int port = getport(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    result_socket_conn conn_result = socket_conn(host.data(),port);
    if(conn_result.state == 1) data_sock = conn_result._socket;

    return conn_result.state;
}


int FTPManager::logoutserver(){
    sprintf(sendBuf,"QUIT \r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    closesocket(data_sock);
    closesocket(control_sock);
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

//解析目录信息
int FTPManager::anaydir() {
    return 1;
}
