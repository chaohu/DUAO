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

    if(!socket_conn(&control_sock,port)) {
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
    if(!setpassmode()) {
        result.state = 0;
        return result;
    }

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
    closesocket(data_sock);
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
    control_sock = socket(AF_INET,SOCK_STREAM,0);

    return 1;
}

int FTPManager::send_order() {
    return 1;
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


    mode_flag = 1;	//主动模式
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
    mode_flag = 0;	//被动模式
    data_sock = socket(AF_INET,SOCK_STREAM,0);
    return socket_conn(&data_sock,port);
}

//改变服务器工作目录
int FTPManager::ch_server_dir(string path) {
    if(path == "..") sprintf(sendBuf,"CDUP\r\n");
    else sprintf(sendBuf,"CWD %s\r\n",path.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    return 1;
}

int FTPManager::file_download(string filename) {
    if(mode_flag) return file_download_act(filename);
    else return file_download_pas(filename);
}

//主动模式下载
int FTPManager::file_download_act(string filename) {
    int flag = 0;
    int state_code;

    sprintf(sendBuf,"SIZE %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) return 0;

    sprintf(sendBuf,"TYPE I\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    if(!setactvmode()) return 0;

    sprintf(sendBuf,"PORT 127,0,0,1,%d,%d\r\n",act_port_part1,act_port_part2);
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    sprintf(sendBuf,"RETR %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d ",&state_code);
//    string message(recvBuf);
//    qDebug(message.substr(message.find('\n',0)+1).data());
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
//    if(state_code1 != 150/* || state_code2 != 226*/) {
//        return 0;
//    }
    if(state_code != 150) return 0;
//    if(message.find('\n',message.find('\n',0)+1) != string::npos) {
//        qDebug("wocha");
//        return 0;
//    }

    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d ",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 226) return 0;

    SOCKET file_socket = socket(AF_INET,SOCK_STREAM,0);
    if(socket_accept(&file_socket)) {
        qDebug("wolai");
        flag = writetofile(file_socket,filename.data());
        closesocket(file_socket);
    }
    closesocket(data_sock);


    return flag;
}

//被动模式从服务器下载
int FTPManager::file_download_pas(string filename) {
    int flag = 0;
    int state_code;

    sprintf(sendBuf,"SIZE %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 213) return 0;

    sprintf(sendBuf,"TYPE I\r\n");
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 200) return 0;

    if(!setpassmode()) return 0;

    sprintf(sendBuf,"RETR %s\r\n",filename.data());
    send(control_sock,sendBuf,strlen(sendBuf),0);
    recv(control_sock,recvBuf,195,0);
    sscanf(recvBuf,"%d",&state_code);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));
    if(state_code != 150) return 0;

    flag = writetofile(data_sock,filename.data());
    closesocket(data_sock);

    recv(control_sock,recvBuf,195,0);
    qDebug(recvBuf);
    memset(recvBuf,0,sizeof(recvBuf));

    return flag;
}


//客户端创建文件
int FTPManager::writetofile(SOCKET _socket,const char *filename) {
    FILE *file;
    qDebug(filename);
    QString filename_gbk(filename);
    if((file = fopen(filename_gbk.toLocal8Bit(),"wb")) == NULL) {
        qDebug("创建文件失败");
        while(recv(_socket,recvBuf,195,0) > 0);
        memset(recvBuf,0,sizeof(recvBuf));
        return 0;
    }
    else {
        qDebug("创建文件成功");
        while(recv(_socket,recvBuf,195,0) > 0) {
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

SOCKET FTPManager::getdatasock() {
    mode_flag = 0;
    return data_sock;
}
