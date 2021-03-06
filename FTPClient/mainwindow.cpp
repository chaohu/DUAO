#include "heartconn.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>

extern QList<unsigned> size_now_no;
extern QMutex mutex_process;

QMutex mutex_exit;
bool conn_exit;

FTPManager *ftpmanager;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mode->addAction("主动",this,SLOT(setactvmode()));
    ui->mode->addAction("被动",this,SLOT(setpassmode()));
    ui->state->addAction("上传下载列表",this,SLOT(showprogressbar()));
    ftpmanager = new FTPManager(this);
    setAcceptDrops(true);

    //上传下载停靠窗口
    progressbar_list = new QDockWidget(tr("上传下载列表"),this);
    progressbar_list->setFeatures(QDockWidget::DockWidgetClosable);
    progressbar_list->setAllowedAreas(Qt::RightDockWidgetArea);
    progressbar_layout = new QVBoxLayout();
    QWidget *in_progressbar_list= new QWidget();
    _progressbar_layout = new QVBoxLayout();
    QPushButton *clear_du_list = new QPushButton("清除列表");
    connect(clear_du_list,SIGNAL(clicked(bool)),this,SLOT(clear_downup_list()));
    _progressbar_layout->setMenuBar(clear_du_list);
    _progressbar_layout->addLayout(progressbar_layout);
    _progressbar_layout->addStretch();
    in_progressbar_list->setLayout(_progressbar_layout);
    progressbar_list->setWidget(in_progressbar_list);
    addDockWidget(Qt::RightDockWidgetArea,progressbar_list);
    progressbar_list->setVisible(false);

    //主体布局
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
    QVBoxLayout *bottomlayout_left = new QVBoxLayout();
    QLabel *local_dir = new QLabel("本地目录列表:");
    bottomlayout_left->addWidget(local_dir);
    QHBoxLayout *local_path_show = new QHBoxLayout();
    local_path = new QLineEdit();
    QString current_local_path = QDir::currentPath();
    local_path->setText(current_local_path);
    local_path_show->addWidget(local_path);
    QPushButton *b_ch_local_dir = new QPushButton("确定");
    connect(b_ch_local_dir,SIGNAL(clicked()),this,SLOT(f_ch_local_dir()));
    local_path_show->addWidget(b_ch_local_dir);
    bottomlayout_left->addLayout(local_path_show);
    locallist = new QListView();
    localstandardItemModel = new QStandardItemModel();
    analysis_local_dir(current_local_path);
    connect(locallist,SIGNAL(clicked(QModelIndex)),this,SLOT(localitemClicked(QModelIndex)));
    bottomlayout_left->addWidget(locallist);

    //底部右侧布局
    QVBoxLayout *bottomlayout_right = new QVBoxLayout();
    QLabel *server_dir = new QLabel("服务器目录列表:");
    bottomlayout_right->addWidget(server_dir);

    QHBoxLayout *server_path_show = new QHBoxLayout();
    server_path = new QLineEdit();
    server_path_show->addWidget(server_path);
    QPushButton *b_ch_server_dir = new QPushButton("确定");
    connect(b_ch_server_dir,SIGNAL(clicked()),this,SLOT(f_ch_server_dir()));
    server_path_show->addWidget(b_ch_server_dir);
    bottomlayout_right->addLayout(server_path_show);

    serverlist = new QListView();
    serverstandardItemModel = new QStandardItemModel();
    QStandardItem *server_init_qsitem = new QStandardItem("当前未连接任何服务器");
    serverstandardItemModel->appendRow(server_init_qsitem);
    serverlist->setModel(serverstandardItemModel);
    connect(serverlist,SIGNAL(clicked(QModelIndex)),this,SLOT(serveritemClicked(QModelIndex)));
    bottomlayout_right->addWidget(serverlist);


    //底部布局
    QHBoxLayout *bottomlayout = new QHBoxLayout();
    bottomlayout->addLayout(bottomlayout_left);
    bottomlayout->addLayout(bottomlayout_right);
    mainlayout->addLayout(bottomlayout);

    //日志布局
    log_message_list = new QListView();
    logstandardItemModel = new QStandardItemModel();
    mainlayout->addWidget(log_message_list);

    mainlayout->setStretchFactor(toplayout,1);
    mainlayout->setStretchFactor(state_info,1);
    mainlayout->setStretchFactor(bottomlayout,12);
    mainlayout->setStretchFactor(log_message_list,4);
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
    if(host.empty() || port.isEmpty()) {
        qDebug("host/port不能为空");
        state_info->setText("连接状态：连接失败");
        return 0;
    }
    else {
        if(username.empty()) username.append("anonymous");
        if(ftpmanager->loginserver(host,username,password,port.toInt())) {
            HeartConn *heartconn = new HeartConn(this,ftpmanager,host,username,password,port.toInt());
            heartconn->start();
            ftpmanager->setmode(0);
            if(ftpmanager->get_dir_list()) {
                if(analysis_server_dir(ftpmanager->server_dir_list_info)) {
                    string temp = "连接状态：连接至";
                    temp.append(host.data());
                    state_info->setText(temp.data());
                    if(ftpmanager->get_server_current_path()) {
                        server_path->setText(ftpmanager->server_current_path);
                    }
                    return 1;
                }
                else {
                    state_info->setText("连接状态：解析目录文件列表失败");
                    return 0;
                }
            }
            else {
                state_info->setText("连接状态：获取目录文件列表失败");
                return 0;
            }
        }
        else {
            state_info->setText("连接状态：连接失败");
            return 0;
        }
    }
}


//设置为主动模式
int MainWindow::setactvmode() {
    if(ftpmanager->getmode() == 1) {
        qDebug("当前正在主动模式");
        return 0;
    }
    else {
        ftpmanager->setmode(1);
        qDebug("成功切换至主动模式");
        return 1;
    }
}

//设置为被动模式
int MainWindow::setpassmode() {
    if(ftpmanager->getmode() == 0) {
        qDebug("当前正在被动模式");
        return 0;
    }
    else {
        ftpmanager->setmode(0);
        qDebug("成功切换至被动模式");
        return 1;
    }
}

//显示上传下载进度条
void MainWindow::showprogressbar() {
    qDebug("显示进度条");
    progressbar_list->setVisible(true);
}

void MainWindow::f_ch_local_dir() {
    QDir::setCurrent(local_path->text());
    flash_local_dir_list();
    qDebug("改变本地目录");
}

void MainWindow::f_ch_local_dir(QString dir_path) {
    QDir::setCurrent(dir_path);
    flash_local_dir_list();
    qDebug("改变本地目录");
}

void MainWindow::f_ch_server_dir() {
    qDebug("改变远端目录");
    if(ftpmanager->ch_server_dir(server_path->text().toStdString())) {
        if(ftpmanager->get_dir_list()) {
            if(analysis_server_dir(ftpmanager->server_dir_list_info)) {

            }
        }
    }
}

int MainWindow::analysis_local_dir(QString local_dir_path) {
    localstandardItemModel->clear();
    QDir dir(local_dir_path);
    dir.setFilter(QDir::AllEntries|QDir::NoDot);
    QFileInfoList local_dir_list = dir.entryInfoList();

    for(int i = 0;i < local_dir_list.size();i++) {
        qDebug() << local_dir_list.at(i).fileName();
        QString dir_name;
        if(local_dir_list.at(i).isDir()) dir_name = "d: ";
        else dir_name = "f: ";
        dir_name.append(local_dir_list.at(i).fileName().toStdString().data());
        QStandardItem *temp = new QStandardItem(dir_name);
        localstandardItemModel->appendRow(temp);
    }
    locallist->setModel(localstandardItemModel);
    return 1;
}

/*
 * 函数功能：解析文件目录信息并显示
 * 注意：注意测试此处是否需要+1,证实需要
 */
int MainWindow::analysis_server_dir(string server_dir_info) {
    dir_info_list.clear();
    serverstandardItemModel->clear();
    QStandardItem *server_parent_dir = new QStandardItem("d: ..");
    serverstandardItemModel->appendRow(server_parent_dir);
    int i = 1;	//文件目录编号从1开始，0留给".."
    unsigned int first = 0;
    unsigned int last = 0;
    while((last = server_dir_info.find('\n',last)) != string::npos) {
        dir_list temp;
        temp.num = i;
        sscanf(server_dir_info.substr(first,last).data(),"%s%d%s%s%d%s%s%s%s",temp.authority,&(temp.node),temp.user,temp.group,&(temp.size),temp.mouth,temp.day,temp.ntime,temp.name);
        QString dir_name;
        if(temp.authority[0] == 'd') dir_name = "d: ";
        else dir_name = "f: ";
        dir_name.append(temp.name);
        QStandardItem *item = new QStandardItem(dir_name);
        serverstandardItemModel->appendRow(item);
        dir_info_list.push_back(temp);
        first = ++last;
        i++;
    }
    serverlist->setModel(serverstandardItemModel);

    return 1;
}


int MainWindow::logoutserver() {
    if(ftpmanager->logoutserver()) {
        serverstandardItemModel->clear();
        QStandardItem *server_init_qsitem = new QStandardItem("当前未连接任何服务器");
        serverstandardItemModel->appendRow(server_init_qsitem);
        serverlist->setModel(serverstandardItemModel);
        state_info->setText("连接状态：已断开");
        server_path->setText("");
        logstandardItemModel->clear();
        log_message_list->setModel(logstandardItemModel);
        mutex_exit.lock();
        conn_exit = true;
        mutex_exit.unlock();
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
    char type = index.data().toString().toStdString().data()[0];
    if(type == 'd') {
        f_ch_local_dir(index.data().toString().mid(3));
    }
    else if(type == 'f') {
        if(ftpmanager->getmode()) ftpmanager->file_upload_act(index.data().toString().mid(3));
        else ftpmanager->file_upload_pas(index.data().toString().mid(3));
//        if(flag) {
//            if(ftpmanager->get_dir_list()) {
//                if(analysis_server_dir(ftpmanager->server_dir_list_info)) {

//                }
//            }
//        }
    }
}

void MainWindow::serveritemClicked(QModelIndex index) {
    qDebug()<<index.data().toString();
    char type = index.data().toString().toStdString().data()[0];
    if(type == 'd') {
        if(ftpmanager->ch_server_dir(index.data().toString().toStdString().substr(3))) {
            if(ftpmanager->get_dir_list()) {
                if(analysis_server_dir(ftpmanager->server_dir_list_info)) {
                    if(ftpmanager->get_server_current_path()) {
                        server_path->setText(ftpmanager->server_current_path);
                    }
                }
            }
        }
    }
    else if(type == 'f'){
        if(ftpmanager->getmode()) ftpmanager->file_download_act(index.data().toString().mid(3));
        else ftpmanager->file_download_pas(index.data().toString().mid(3));
    }
}


void MainWindow::check_file(bool check) {
    if(check) {
        QMessageBox::warning(this,tr("提示"),tr("下载文件校验正确！"),QMessageBox::Yes);
    }
    else {
        QMessageBox::warning(this,tr("提示！"),tr("下载文件校验失败！"),QMessageBox::Yes);
    }
}

void MainWindow::file_uploaded(QString filename) {
    QMessageBox::warning(this,tr("提示"),filename+"文件上传完成！",QMessageBox::Yes);
    flash_server_dir_list();
}

void MainWindow::file_downloaded(QString filename) {
    QMessageBox::warning(this,tr("提示"),filename+"文件下载完成！",QMessageBox::Yes);
    flash_local_dir_list();
}


void MainWindow::flash_local_dir_list() {
    localstandardItemModel->clear();
    QDir dir(QDir::currentPath());
    dir.setFilter(QDir::AllEntries|QDir::NoDot);
    QFileInfoList local_dir_list = dir.entryInfoList();

    for(int i = 0;i < local_dir_list.size();i++) {
        qDebug() << local_dir_list.at(i).fileName();
        QString dir_name;
        if(local_dir_list.at(i).isDir()) dir_name = "d: ";
        else dir_name = "f: ";
        dir_name.append(local_dir_list.at(i).fileName().toStdString().data());
        QStandardItem *temp = new QStandardItem(dir_name);
        localstandardItemModel->appendRow(temp);
    }
    locallist->setModel(localstandardItemModel);
}

void MainWindow::flash_server_dir_list() {
    if(ftpmanager->get_dir_list()) {
        string server_dir_info = ftpmanager->server_dir_list_info;
        dir_info_list.clear();
        serverstandardItemModel->clear();
        QStandardItem *server_parent_dir = new QStandardItem("d: ..");
        serverstandardItemModel->appendRow(server_parent_dir);
        int i = 1;	//文件目录编号从1开始，0留给".."
        unsigned int first = 0;
        unsigned int last = 0;
        while((last = server_dir_info.find('\n',last)) != string::npos) {
            dir_list temp;
            temp.num = i;
            sscanf(server_dir_info.substr(first,last).data(),"%s%d%s%s%d%s%s%s%s",temp.authority,&(temp.node),temp.user,temp.group,&(temp.size),temp.mouth,temp.day,temp.ntime,temp.name);
            QString dir_name;
            if(temp.authority[0] == 'd') dir_name = "d: ";
            else dir_name = "f: ";
            dir_name.append(temp.name);
            QStandardItem *item = new QStandardItem(dir_name);
            serverstandardItemModel->appendRow(item);
            dir_info_list.push_back(temp);
            first = ++last;
            i++;
        }
        serverlist->setModel(serverstandardItemModel);
    }
}


//添加上传下载进度条
void MainWindow::add_progressbar(int num ,QString filename) {
    qDebug()<<(new QString("progressbar"));
    du_progressbar *_du_progressbar = new du_progressbar();
    _du_progressbar->filename = new QLabel(filename);
    _du_progressbar->progressbar = new QProgressBar();
    _du_progressbar->progressbar->setRange(0,100);
    _du_progressbar->progressbar->setValue(1);
    while(progressbar.size()<(unsigned)num+1) progressbar.push_back(NULL);
    progressbar[num] = _du_progressbar;
    progressbar_layout->addWidget(_du_progressbar->filename);
    progressbar_layout->addWidget(_du_progressbar->progressbar);
}


//刷新上传下载进度条
void MainWindow::flash_bar(int num, unsigned value) {
    qDebug("flash");
    progressbar[num]->progressbar->setValue(value);
}


//清空下载上传进度条
void MainWindow::clear_downup_list() {
    for(int i = 0;i < (int)progressbar.size();i++) {
        progressbar_layout->removeWidget(progressbar[i]->filename);
        progressbar_layout->removeWidget(progressbar[i]->progressbar);
        delete(progressbar[i]->filename);
        delete(progressbar[i]->progressbar);
    }
    progressbar.clear();
    mutex_process.lock();
    size_now_no.clear();
    mutex_process.unlock();
}


//添加log信息
void MainWindow::add_log(QString log_info) {
    QStandardItem *item = new QStandardItem(log_info);
    logstandardItemModel->appendRow(item);
    log_message_list->setModel(logstandardItemModel);
    log_message_list->scrollToBottom();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    //如果为文件支持拖放
    if(event->mimeData()->hasFormat("text/uri-list")) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty()) return;
    else {
        foreach (QUrl url, urls) {
            int site = url.toLocalFile().lastIndexOf('/');
            char hehe[10];
            itoa(site,hehe,10);
            qDebug(hehe);
            f_ch_local_dir(url.toLocalFile().left(site));
            if(ftpmanager->getmode()) ftpmanager->file_upload_act(url.toLocalFile().mid(site+1));
            else ftpmanager->file_upload_pas(url.toLocalFile().mid(site+1));
            flash_server_dir_list();
        }
    }
}









//断开重连代码
HeartConn::HeartConn(MainWindow *mainwindow,FTPManager *ftpmanager, string host, string username, string password, int port) {
    HeartConn::mainwindow = mainwindow;
    HeartConn::ftpmanager = ftpmanager;
    HeartConn::control_sock = ftpmanager->getcontrolsock();
    HeartConn::host = host;
    HeartConn::username = username;
    HeartConn::password = password;
    HeartConn::port = port;
    connect(this,SIGNAL(flash_server_dir()),mainwindow,SLOT(flash_server_dir_list()));
}

HeartConn::~HeartConn() {

}

void HeartConn::run() {
    int conn_state = 0;
    char sendBuf[20] = "\r\n";
    char temp[200];
    bool exit = false;
    while(true) {
        mutex_exit.lock();
        exit = conn_exit;
        mutex_exit.unlock();
        if(exit) break;
        else {
            conn_state = send(control_sock,sendBuf,strlen(sendBuf),0);
            itoa(conn_state,temp,10);
            qDebug(temp);
            if(conn_state == -1) {
                if(ftpmanager->loginserver(host,username,password,port)) {
                    control_sock = ftpmanager->getcontrolsock();
                    emit flash_server_dir();
                }
            }
        }
        msleep(2000);
    }
    mutex_exit.lock();
    conn_exit = false;
    mutex_exit.unlock();
}
