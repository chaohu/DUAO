#ifndef FTPMANAGER_H
#define FTPMANAGER_H
#include <winsock2.h>
#include <string>
using namespace std;

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

    //登陆服务器

    int loginserver(const std::string host,const std::string username,const std::string password,const int port);
    int setactvmode();
    int setpassmode();
    int logoutserver();
};

#endif // FTPMANAGER_H
