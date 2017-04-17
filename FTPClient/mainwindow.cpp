#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPushButton *btn = new QPushButton(this);
    QLineEdit *le = new QLineEdit(this);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(btn,0,0,1,1);
    layout->addWidget(le,0,1,1,3);
    ui->centralWidget->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}
