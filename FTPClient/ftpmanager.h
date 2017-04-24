#ifndef FTPMANAGER_H
#define FTPMANAGER_H
#include <winsock2.h>
#include <string>
using std::string;


struct result_login {
    int state;
    string server_dir_info;
};

class FTPManager
{
private:
    string host;
    int act_port_part1;
    int act_port_part2;
    int mode_flag;	//主被动模式标记：0被动，1主动

    char sendBuf[20];
    char recvBuf[200];

    SOCKET control_sock;
    SOCKET data_sock;


public:
    FTPManager();

    ~FTPManager();

    //主函数
    result_login loginserver(const std::string host,const std::string username,const std::string password,const int port);//登陆服务器
    int init_sock();//初始化socket资源
    int socket_conn(SOCKET *_socket,const int port);	//创建一个socket连接
    int socket_accept(SOCKET *_socket);//接受一个socket连接
    int setactvmode();//设置主动态
    int setpassmode();//设置被动态
    int ch_server_dir(string path);//改变服务器工作目录
    int file_download(string filename);
    int file_download_act(string filename);//主动模式从服务器下载文件
    int file_download_pas(string filename);//被动模式从服务器下载文件
    int writetofile(SOCKET _socket,const char *filename);//将文件写到本地磁盘
    int logoutserver();//断开服务器连接

    //辅助函数
    int getport(const char *recvBuf);
    int getmode();//获取当前模式
    SOCKET getdatasock();//获取数据socket
};

#endif // FTPMANAGER_H
