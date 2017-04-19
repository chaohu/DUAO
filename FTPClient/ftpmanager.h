#ifndef FTPMANAGER_H
#define FTPMANAGER_H
#include <winsock2.h>
#include <string>
using namespace std;

class FTPManager
{
private:
    SOCKET sockClient;

public:
    FTPManager();

    ~FTPManager();

    //登陆服务器

    int loginserver(const std::string host,const std::string username,const std::string password,const int port);
    int logoutserver();
};

#endif // FTPMANAGER_H
