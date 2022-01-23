#include "client.h"
#include "ui_client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    m_socket = new QTcpSocket();

    connect(ui->b_register, &QPushButton::clicked, this, &Client::onRegisterClicked);
    connect(ui->b_logIn, &QPushButton::clicked, this, &Client::onLogInClicked);
    connect(ui->e_IP, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);
    connect(ui->sb_port, &QSpinBox::textChanged, ui->statusbar, &QStatusBar::clearMessage);

    connect(ui->e_login, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);
    connect(ui->e_password, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);

    connect(ui->b_register_2, &QPushButton::clicked, this, &Client::onRegister_2Clicked);
    connect(ui->b_cancel, &QPushButton::clicked, this, &Client::onCancelClicked);
    connect(ui->e_newLogin, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);
    connect(ui->e_newPassword, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);
    connect(ui->e_repPassword, &QLineEdit::textChanged, ui->statusbar, &QStatusBar::clearMessage);

    connect(ui->b_send, &QPushButton::clicked, this, &Client::sendMessage);
    connect(ui->b_leave, &QPushButton::clicked, this, &Client::leaveChatroom);


    connect(m_socket, &QTcpSocket::connected, this, &Client::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::leaveChatroom);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::received);


    openLogInPage();
}

void Client::received()
{
    ui->te_chat->setText(ui->te_chat->toHtml() + "\n" + m_socket->readAll());
    ui->te_chat->verticalScrollBar()->setValue(ui->te_chat->verticalScrollBar()->maximum());
}

void Client::connected()
{
    isClientConnected = true;
    if (auth() == true)
        openChatroomPage();
    else if (ui->statusbar->currentMessage().isEmpty()) {
        ui->statusbar->showMessage("not authorized.");
    }
    else {
        isClientConnected = false;
        abortConnection();
    }
}

void Client::openLogInPage()
{
    ui->e_login->clear();
    ui->e_password->clear();

    ui->stackedWidget->setCurrentIndex(0);
}

void Client::openRegistrationPage()
{
    ui->e_newLogin->clear();
    ui->e_newPassword->clear();
    ui->e_repPassword->clear();

    ui->stackedWidget->setCurrentIndex(1);
}

void Client::openChatroomPage()
{
    ui->stackedWidget->setCurrentIndex(2);
}

bool Client::checkConnection()
{
    if (ui->e_IP->text().isEmpty() == false) {
        if (isClientConnected) {
            return true;
        }
        QString host = ui->e_IP->text();
        int port = ui->sb_port->value();
        m_socket->connectToHost(host, port);
        return true;
    }

    ui->statusbar->showMessage("unable to connect. check the IP and port or try again later.");
    return false;
}

void Client::abortConnection()
{
    isClientConnected = false;
    m_socket->disconnectFromHost();
}

void Client::onRegisterClicked()
{
    if (checkConnection() == true) {
        openRegistrationPage();
    }
}

void Client::onLogInClicked()
{
    checkConnection();
}

void Client::onRegister_2Clicked()
{
    if (ui->e_newLogin->text().isEmpty() == false) {
        if (isLoginFree() == true) {
            if (ui->e_newPassword->text().length() < 6 || ui->e_newPassword->text().length() > 32) {
                ui->statusbar->showMessage("the password should be 6 to 32 symbols long.");
            }
            else if (ui->e_newPassword->text() == ui->e_repPassword->text()) {
                createNewUser();
                openLogInPage();
            }
            else {
                ui->statusbar->showMessage("passwords don't match.");
            }
        }
        else {
            ui->statusbar->showMessage("this login is already taken.");
        }
    }
    else {
        ui->statusbar->showMessage("enter the login.");
    }
}

void Client::onCancelClicked()
{
    openLogInPage();
    abortConnection();
}

void Client::leaveChatroom()
{
    if (ui->stackedWidget->currentIndex() == 2) {
        openLogInPage();
        ui->te_message->clear();
    }
    abortConnection();
}

void Client::sendMessage()
{
    m_socket->write(QString("c:::m|" + ui->te_message->toPlainText()).toUtf8());
    ui->te_message->clear();
}

bool Client::isLoginFree()
{
    /*
     *      send the login to the server
     *      if the login is not mentioned in database, return true
     *      else return false
     */
    qDebug() << "let's say login is free.";
    return true;
}

void Client::createNewUser()
{
    /*
     *      send the login and the password to the server
     *      add a new entry to the database
     */
    qDebug() << "sending microwaves to the server...";
}

bool Client::auth()
{
    if (ui->e_login->text().isEmpty() == false) {
        if (ui->e_password->text().isEmpty() == false) {
            bool match = false;

            m_socket->write(QString("c:::l|" + ui->e_login->text()+'\0'+ui->e_password->text()).toUtf8());

            match = true;

            if (match) return true;
            else ui->statusbar->showMessage("wrong login or password. try again.");
        }
        else ui->statusbar->showMessage("enter the password.");
    }
    else ui->statusbar->showMessage("enter the login.");

    return false;
}

Client::~Client()
{
    delete ui;
}

