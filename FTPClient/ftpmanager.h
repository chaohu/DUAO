#ifndef FTPMANAGER_H
#define FTPMANAGER_H
#include <winsock2.h>
#include <string>
using namespace std;

struct result_socket_conn {
    int state;
    SOCKET _socket;
};

struct result_login {
    int state;
    string server_dir_info;
};

class FTPManager
{
private:
    string host;

    char sendBuf[20];
    char recvBuf[200];

    SOCKET control_sock;
    SOCKET data_sock;

public:
    FTPManager();

    ~FTPManager();

    //主函数
    result_login loginserver(const std::string host,const std::string username,const std::string password,const int port);//登陆服务器
    result_socket_conn socket_conn(const char *host,const int port);	//创建一个socket连接
    int setactvmode();//设置主动态
    int setpassmode();//设置被动态
    SOCKET pass_conn(SOCKET control_sock);//被动态连接
    int file_download(string filename);//从服务器下载文件
    int logoutserver();//断开服务器连接

    //辅助函数
    int getport(const char *recvBuf);
    int anaydir();
};

#endif // FTPMANAGER_H
