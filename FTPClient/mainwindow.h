#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dirent.h>
#include <string>
#include <vector>
using namespace std;

#include <QLineEdit>
#include <QLabel>

#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

struct dir_list {
    int num;
    char authority[15];
    unsigned int node;
    char user[15];
    char group[15];
    unsigned int size;
    char mouth[15];
    char day[5];
    char ntime[10];
    char name[50];
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    vector<dir_list> dir_info_list;

private:
    Ui::MainWindow *ui;
    QLineEdit *host_e;
    QLineEdit *username_e;
    QLineEdit *pwd_e;
    QLineEdit *port_e;
    QLabel *state_info;
    QListView *locallist;
    QListView *serverlist;
    QListView *log_message_list;
    QStandardItemModel *localstandardItemModel;
    QStandardItemModel *serverstandardItemModel;
    QStandardItemModel *logstandardItemModel;
private slots:
    int loginserver();
    int setactvmode();
    int setpassmode();
    int analysis_local_dir(QString local_dir_path);
    int analysis_server_dir(string server_dir_info);
    int logoutserver();
    void localitemClicked(QModelIndex index);
    void serveritemClicked(QModelIndex index);
};

#endif // MAINWINDOW_H
