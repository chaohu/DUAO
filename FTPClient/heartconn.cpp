#include <QMutex>
#include <winsock2.h>
#include "heartconn.h"
#include "ftpmanager.h"
#include "mainwindow.h"

extern QMutex mutex_exit;
extern bool conn_exit;

HeartConn::HeartConn(MainWindow *mainwindow,FTPManager *ftpmanager,SOCKET control_sock, string host, string username, string password, int port) {
    HeartConn::mainwindow = mainwindow;
    HeartConn::ftpmanager = ftpmanager;
    HeartConn::control_sock = control_sock;
    HeartConn::host = host;
    HeartConn::username = username;
    HeartConn::password = password;
    HeartConn::port = port;
    connect(this,SIGNAL(flash_server_dir()),mainwindow,SLOT(flash_server_dir_list()));
}

HeartConn::~HeartConn() {

}

void HeartConn::run() {
    char temp[200];
    bool exit = false;
    while(true) {
        mutex_exit.lock();
        exit = conn_exit;
        mutex_exit.unlock();
        if(exit) break;
        else {
            send(control_sock,"Type I\r\n",15,0);
            if(recv(control_sock,temp,20,0) <= 0) {
                if(ftpmanager->loginserver(host,username,password,port)) emit flash_server_dir();
            }
        }
        msleep(500);
    }
}
