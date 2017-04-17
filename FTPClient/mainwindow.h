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
    void localitemClicked(QModelIndex index);
    void serveritemClicked(QModelIndex index);
};

#endif // MAINWINDOW_H
