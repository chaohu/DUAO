#ifndef FTPMANAGER_H
#define FTPMANAGER_H
#include "downloadthread.h"
#include "uploadthread.h"
#include <winsock2.h>
#include <string>
using std::string;


class FTPManager
{
private:
    MainWindow *mainwindow;
    string host;
    int local_ip_part1;
    int local_ip_part2;
    int local_ip_part3;
    int local_ip_part4;
    int act_port_part1;
    int act_port_part2;
    int mode_flag;	//主被动模式标记：0被动，1主动

    char sendBuf[20];
    char recvBuf[200];

    SOCKET control_sock;
    SOCKET data_sock;

public:
    char server_current_path[260];//服务器当前工作目录
    string server_dir_list_info;//服务器目录文件列表信息

public:
    FTPManager(MainWindow *mainwindow);

    ~FTPManager();

    //主函数
    int loginserver(const std::string host,const std::string username,const std::string password,const int port);//登陆服务器
    int init_sock();//初始化socket资源
    int send_order(string order);//发送命令
    int socket_conn(SOCKET *_socket,const int port);	//创建一个socket连接
    int socket_accept(SOCKET *_socket);//接受一个socket连接
    int setactvmode();//设置主动态
    int setpassmode();//设置被动态
    int get_server_current_path();//获取服务器当前工作目录
    int ch_server_dir(string path);//改变服务器工作目录
    int get_dir_list();//获取当前目录文件列表
    int file_download_act(QString filename);//主动模式从服务器下载文件
    int file_download_pas(QString filename);//被动模式从服务器下载文件
    int file_upload_act(QString filename);//主动模式上传文件到服务器
    int file_upload_pas(QString filename);//被动模式上传文件到服务器
    int writetofile(SOCKET _socket,const char *filename);//将文件写到本地磁盘
    int readfromfile(SOCKET _socket,const char *filename);//从本地磁盘读取文件
    int logoutserver();//断开服务器连接

    //辅助函数
    int getport(const char *recvBuf);
    int getmode();//获取当前模式
    int setmode(int mode_flag);//设置当前模式
    SOCKET getdatasock();//获取数据socket
};

#endif // FTPMANAGER_H
