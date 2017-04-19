#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QListView *locallist;
    QListView *serverlist;
    QStandardItemModel *localstandardItemModel;
    QStandardItemModel *serverstandardItemModel;
private slots:
    int loginserver(const QString host,const QString username,const QString password,const QString port);
    int logoutserver();
    void localitemClicked(QModelIndex index);
    void serveritemClicked(QModelIndex index);
};

#endif // MAINWINDOW_H
