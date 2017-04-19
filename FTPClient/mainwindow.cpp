#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ftpmanager.h"
#include <string>
using namespace std;

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
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
    host_e = new QLineEdit();
    QLabel *username_l = new QLabel("Username:");
    username_e = new QLineEdit();
    QLabel *pwd_l = new QLabel("Password:");
    pwd_e = new QLineEdit();
    QLabel *port_l = new QLabel("Port:");
    port_e = new QLineEdit();
    port_e->setValidator(new QIntValidator(0,10000,this));
    QPushButton *quickconn = new QPushButton("Quickconnect");
    connect(quickconn,SIGNAL(clicked()),this,SLOT(loginserver()));
    QPushButton *disconn = new QPushButton("Disconnect");
    connect(disconn,SIGNAL(clicked()),this,SLOT(logoutserver()));
    toplayout->addWidget(host_l);
    toplayout->addWidget(host_e);
    toplayout->addWidget(username_l);
    toplayout->addWidget(username_e);
    toplayout->addWidget(pwd_l);
    toplayout->addWidget(pwd_e);
    toplayout->addWidget(port_l);
    toplayout->addWidget(port_e);
    toplayout->addWidget(quickconn);
    toplayout->addWidget(disconn);
    toplayout->addStretch();

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


    mainlayout->addLayout(toplayout);
    mainlayout->addLayout(bottomlayout);
    ui->centralWidget->setLayout(mainlayout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::loginserver() {
    qDebug("hehe");
    qDebug(host_e->text().toStdString().data());
    qDebug(username_e->text().toStdString().data());
    return ftpmanager->loginserver(host_e->text().toStdString(),username_e->text().toStdString(),pwd_e->text().toStdString(),port_e->text().toInt());
}

int MainWindow::setactvmode() {
    return ftpmanager->setactvmode();
}

int MainWindow::setpassmode() {
    return ftpmanager->setpassmode();
}

int MainWindow::logoutserver() {
    return ftpmanager->logoutserver();
}

void MainWindow::localitemClicked(QModelIndex index) {
    qDebug()<<index.data().toString();
}

void MainWindow::serveritemClicked(QModelIndex index) {
    qDebug()<<index.data().toString();
}
