#include "ftpmanager.h"
#include <QDebug>

FTPManager::FTPManager() {
}

FTPManager::~FTPManager() {

}


result_login FTPManager::loginserver(const std::string host,const std::string username,const std::string password,const int port) {
    FTPManager::host = host;
    result_login result;

    if(init_sock() == 0) {
        result.state = 0;
        return result;
    }

    if(socket_conn(&control_sock,host.data(),port) == 0) {
        result.state = 0;
        return result;
    }

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
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    qDebug("LIST返回信息：");
    string server_dir_info = "";
    while(recv(data_sock,recvBuf,195,0) > 0) {
        server_dir_info.append(recvBuf);
        memset(recvBuf,0,sizeof(recvBuf));
    }
    memset(recvBuf,0,sizeof(recvBuf));
    qDebug("%s",server_dir_info.data());
    result.server_dir_info = server_dir_info;

    result.state = 1;
    return result;
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

    //初始化socket
    control_sock = socket(AF_INET,SOCK_STREAM,0);
    data_sock = socket(AF_INET,SOCK_STREAM,0);
    return 1;
}

int FTPManager::socket_conn(SOCKET *_socket,const char *host,const int port) {

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(host);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);


    //连接到服务器端端口
    if(SOCKET_ERROR == connect(*_socket,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR))) {
        if(errno == 0) {
            qDebug("socket已经连接");
            return 2;
        }
        else {
            qDebug("socket连接失败");
            return 0;
        }
    }
    else {
        qDebug("socket连接成功");
        return 1;//正常连接
    }
}

//随机生成的端口范围：1025-65531
int FTPManager::setactvmode() {
    closesocket(data_sock);
    int port_part1 = 0;
    int port_part2 = 0;
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htons(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    do {
        char error_info[10];
        error_info[0] = WSAGetLastError() + '0';
        error_info[1] = '\0';
        qDebug(error_info);
        port_part1 = rand()%251 + 4;
        port_part2 = rand()%250 + 1;
        addrSrv.sin_port = htons(port_part1*4 + port_part2);
    } while(bind(data_sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR)) == SOCKET_ERROR);
    listen(data_sock,64);

    sprintf(sendBuf,"PORT 127.0.0.1,%d,%d\r\n",port_part1,port_part2);
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

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

    return socket_conn(&data_sock,host.data(),port);
}

int FTPManager::file_download(string filename) {
    sprintf(sendBuf,"SIZE %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    sprintf(sendBuf,"TYPE I\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(mode_flag) setactvmode();
    else setpassmode();
    sprintf(sendBuf,"RETR %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    //客户端创建文件
    FILE *file;
    qDebug(filename.data());
    QString filename_gbk(filename.data());
    if((file = fopen(filename_gbk.toLocal8Bit(),"wb")) == NULL) {
        qDebug("创建文件夹失败");
        while(recv(data_sock,recvBuf,195,0) > 0);
        memset(recvBuf,0,sizeof(recvBuf));
        return 0;
    }
    else {
        qDebug("创建文件夹成功");
        while(recv(data_sock,recvBuf,195,0) > 0) {
            fwrite(recvBuf,1,195,file);
            memset(recvBuf,0,sizeof(recvBuf));
        }
        fclose(file);
        return 1;
    }
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
