#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLineEdit>

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
    QLineEdit *host_e;
    QLineEdit *username_e;
    QLineEdit *pwd_e;
    QLineEdit *port_e;
    QListView *locallist;
    QListView *serverlist;
    QStandardItemModel *localstandardItemModel;
    QStandardItemModel *serverstandardItemModel;
private slots:
    int loginserver();
    int setactvmode();
    int setpassmode();
    int logoutserver();
    void localitemClicked(QModelIndex index);
    void serveritemClicked(QModelIndex index);
};

#endif // MAINWINDOW_H
