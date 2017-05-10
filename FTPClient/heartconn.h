#ifndef HEARTCONN_H
#define HEARTCONN_H
#include "ftpmanager.h"
#include <QThread>
#include <string>
using std::string;

class HeartConn : public QThread
{
    Q_OBJECT

signals:
    void flash_server_dir();

private:
    MainWindow *mainwindow;
    FTPManager *ftpmanager;
    SOCKET control_sock;
    string host;
    string username;
    string password;
    int port;

public:
    HeartConn(MainWindow *mainwindow,FTPManager *ftpmanager,string host,string username,string password,int port);
    ~HeartConn();

    void run();

};

#endif // HEARTCONN_H
