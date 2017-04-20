#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ftpmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QDebug>

FTPManager *ftpmanager = new FTPManager();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menu->addAction("主动",this,SLOT(setactvmode()));
    ui->menu->addAction("被动",this,SLOT(setpassmode()));


    //主题布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);

    //顶部布局
    QHBoxLayout *toplayout = new QHBoxLayout();
    QLabel *host_l = new QLabel("Host:");
    toplayout->addWidget(host_l);
    host_e = new QLineEdit();
    toplayout->addWidget(host_e);
    QLabel *username_l = new QLabel("Username:");
    toplayout->addWidget(username_l);
    username_e = new QLineEdit();
    toplayout->addWidget(username_e);
    QLabel *pwd_l = new QLabel("Password:");
    toplayout->addWidget(pwd_l);
    pwd_e = new QLineEdit();
    toplayout->addWidget(pwd_e);
    QLabel *port_l = new QLabel("Port:");
    toplayout->addWidget(port_l);
    port_e = new QLineEdit();
    port_e->setValidator(new QIntValidator(0,10000,this));
    toplayout->addWidget(port_e);
    QPushButton *quickconn = new QPushButton("Quickconnect");
    connect(quickconn,SIGNAL(clicked()),this,SLOT(loginserver()));
    toplayout->addWidget(quickconn);
    QPushButton *disconn = new QPushButton("Disconnect");
    connect(disconn,SIGNAL(clicked()),this,SLOT(logoutserver()));
    toplayout->addWidget(disconn);
    toplayout->addStretch();
    mainlayout->addLayout(toplayout);

    //连接状态
    state_info = new QLabel("连接状态:无连接");
    state_info->setFrameShape(QFrame::Box);
    mainlayout->addWidget(state_info);

    //底部左侧布局
    locallist = new QListView();
    localstandardItemModel = new QStandardItemModel();
    QStringList localstrlist;
    localstrlist.append("hehe");
    localstrlist.append("wowo");
    localstrlist.append("qpqp");
    for(int i = 0;i < localstrlist.size();i++) {
        QString localstring = static_cast<QString>(localstrlist.at(i));
        QStandardItem *localitem = new QStandardItem(localstring);
        localstandardItemModel->appendRow(localitem);
    }
    locallist->setModel(localstandardItemModel);
    connect(locallist,SIGNAL(clicked(QModelIndex)),this,SLOT(localitemClicked(QModelIndex)));

    //底部右侧布局
    serverlist = new QListView();
    serverstandardItemModel = new QStandardItemModel();
    QStringList serverstrlist;
    serverstrlist.append("hehe");
    serverstrlist.append("wowo");
    serverstrlist.append("qpqp");
    for(int i = 0;i < serverstrlist.size();i++) {
        QString serverstring = static_cast<QString>(serverstrlist.at(i));
        QStandardItem *serveritem = new QStandardItem(serverstring);
        serverstandardItemModel->appendRow(serveritem);
    }
    serverlist->setModel(serverstandardItemModel);
    connect(serverlist,SIGNAL(clicked(QModelIndex)),this,SLOT(serveritemClicked(QModelIndex)));

    //底部布局
    QHBoxLayout *bottomlayout = new QHBoxLayout();
    bottomlayout->addWidget(locallist);
    bottomlayout->addWidget(serverlist);


    mainlayout->addLayout(bottomlayout);
    ui->centralWidget->setLayout(mainlayout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::loginserver() {
    state_info->setText("连接状态：连接中");
    std::string host = host_e->text().toStdString();
    std::string	username = username_e->text().toStdString();
    std::string password = pwd_e->text().toStdString();
    QString port = port_e->text();
    if(host.empty() || username.empty() || password.empty() || port.isEmpty()) {
        qDebug("host/username/password/port不能为空");
        state_info->setText("连接状态：连接失败");
        return 0;
    }
    else {
        result_login result = ftpmanager->loginserver(host,username,password,port.toInt());
        if(result.state == 1) {
            analysis_dir(result.dir_info);
            string temp = "连接状态：连接至";
            temp.append(host.data());
            state_info->setText(temp.data());
            return 1;
        }
        else if(result.state == 2) {
            state_info->setText("连接状态：重复连接");
            return 2;
        }
        else {
            state_info->setText("连接状态：连接失败");
            return 0;
        }
    }
}

int MainWindow::setactvmode() {
    if(ftpmanager->setactvmode()) {
        qDebug("成功切换至主动模式");
        return 1;
    }
    else {
        qDebug("切换主动模式失败");
        return 0;
    }
}

int MainWindow::setpassmode() {
    int state = ftpmanager->setpassmode();
    if(state == 1) {
        state_info->setText("成功切换至被动模式");
        qDebug("成功切换至被动模式");
        return 1;
    }
    else if(state == 2) {
        state_info->setText("当前正在被动模式");
        qDebug("当前正在被动模式");
        return 2;
    }
    else {
        state_info->setText("切换被动模式失败");
        qDebug("切换被动模式失败");
        return 0;
    }
}


//注意测试此处是否需要+1
int MainWindow::analysis_dir(string dir_info) {
    dir_info_list.clear();
    int i = 0;
    unsigned int first = 0;
    unsigned int last = 0;
    while((last = dir_info.find('\r\n',last)) != string::npos) {
        dir_list temp;
        temp.num = i;
        sscanf(dir_info.substr(first,last),"%s%d%s%s%d%s%s%s%s",temp.authority,&(temp.node),temp.name,
               temp.group,&(temp.size),temp.mouth,temp.day,temp.ntime,temp.name);
        QString dir_name = temp.name;
        QStandardItem *item = new QStandardItem(dir_name);
        serverstandardItemModel->appendRow(item);
        dir_info_list.push_back(temp);
        first = last;
        i++;
    }
    dir_list temp;
    temp.num = i;
    sscanf(dir_info.substr(first,last),"%s%d%s%s%d%s%s%s%s",temp.authority,&(temp.node),temp.name,
           temp.group,&(temp.size),temp.mouth,temp.day,temp.ntime,temp.name);
    QString dir_name = temp.name;
    QStandardItem *item = new QStandardItem(dir_name);
    serverstandardItemModel->appendRow(item);
    dir_info_list.push_back(temp);
    serverlist->setModel(serverstandardItemModel);

    return 1;
}


int MainWindow::logoutserver() {
    if(ftpmanager->logoutserver()) {
        state_info->setText("连接状态：已断开");
        return 0;
    }
    else {
        string temp = "连接状态：断开失败 连接至";
        temp.append(host_e->text().toStdString().data());
        state_info->setText(temp.data());
        return 0;
    }
}

void MainWindow::localitemClicked(QModelIndex index) {
    qDebug()<<index.data().toString();
}

void MainWindow::serveritemClicked(QModelIndex index) {
    qDebug()<<index.data().toString();
}
