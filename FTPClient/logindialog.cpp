#include "logindialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    usrLabel = new QLabel(this);
    usrLabel->move(70,80);
    usrLabel->setText(tr("用户名："));
    usrLineEdit = new QLineEdit(this);
    usrLineEdit->move(140,80);
    usrLineEdit->setPlaceholderText(tr("请输入用户名"));

    pwdLabel = new QLabel(this);
    pwdLabel->move(70,130);
    pwdLabel->setText(tr("密码："));
    pwdLineEdit = new QLineEdit(this);
    pwdLineEdit->move(140,130);
    pwdLineEdit->setPlaceholderText(tr("请输入密码"));

    loginBtn = new QPushButton(this);
    loginBtn->move(50,200);
    loginBtn->setText(tr("登陆"));
    exitBtn = new QPushButton(this);
    exitBtn->move(210,200);
    exitBtn->setText(tr("退出"));

    connect(loginBtn,&QPushButton::clicked,this,&LoginDialog::login);
    connect(exitBtn,&QPushButton::clicked,this,&LoginDialog::close);
}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::login() {
    if(usrLineEdit->text().trimmed() == tr("huchao") && pwdLineEdit->text() == tr("123456")) {
        accept();
    }
    else {
        QMessageBox::warning(this,tr("警告！"),tr("用户名或密码错误！"),QMessageBox::Yes);

        usrLineEdit->clear();
        pwdLineEdit->clear();
        usrLineEdit->setFocus();
    }
}
