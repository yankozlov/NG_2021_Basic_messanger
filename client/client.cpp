#include "client.h"
#include "ui_client.h"
#include "encription.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    m_socket = new QTcpSocket();

    model = new QStringListModel();
    ui->lv_usersOnline->setModel(model);

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
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::abortConnection);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::received);

    openLogInPage();
}

void Client::received()
{
    QByteArray receivedData = m_socket->readAll();

    if (receivedData.indexOf("s:::r|Permitted.") == 0) {
        receivedData.remove(0, QString("s:::r|Permitted.").length());

        openLogInPage();
        abortConnection();
    }
    else if (receivedData.indexOf("s:::r|Forbidden.") == 0) {
        receivedData.remove(0, QString("s:::r|Forbidden.").length());

        ui->statusbar->showMessage("this login is alredy taken.");
    }
    else if (receivedData.indexOf("s:::l|Permitted.") == 0) {
        receivedData.remove(0, QString("s:::l|Permitted.").length());

        openChatroomPage();
    }
    else if (receivedData.indexOf("s:::l|Forbidden.") == 0) {
        receivedData.remove(0, QString("s:::l|Forbidden.").length());

        ui->statusbar->showMessage("wrong login or password. try again.");
        abortConnection();
    }
    if (receivedData.indexOf("s:::u|") == 0) {
        refreshUsersList(receivedData);
    }
    else if (receivedData.indexOf("s:::m|") == 0) {
        //set limit for chat size

        receivedData.remove(0, QString("s:::m|").length());

        receivedData = (ui->e_login->text() + ": " + decript(QString(receivedData.remove(0, ui->e_login->text().length()+2)))).toUtf8();

        ui->te_chat->setText(ui->te_chat->toHtml() + "\n" + receivedData);
        ui->te_chat->verticalScrollBar()->setValue(ui->te_chat->verticalScrollBar()->maximum());
    }
}

void Client::connected()
{
    ui->statusbar->clearMessage();
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
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            return true;
        }
        QString host = ui->e_IP->text();
        int port = ui->sb_port->value();
        m_socket->connectToHost(host, port);
        m_socket->waitForConnected(500);
        if (m_socket->state() == QAbstractSocket::ConnectedState)
            return true;
        else {
            ui->statusbar->showMessage("unable to connect. check the IP and port or try again later.");
            abortConnection();
            return false;
        }
    }
    else {
        ui->statusbar->showMessage("enter the IP address.");
        return false;
    }
}

void Client::abortConnection()
{    
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        m_socket->abort();
    }
    m_socket->disconnectFromHost();
    if (ui->stackedWidget->currentIndex() == 2) {
        leaveChatroom();
        ui->statusbar->showMessage("connection dropped.");
    }
}

void Client::onRegisterClicked()
{
    if (checkConnection() == true) {
        openRegistrationPage();
    }
}

void Client::onLogInClicked()
{
    if (ui->e_IP->text().isEmpty() == false) {
        if (ui->e_login->text().isEmpty() == false) {
            if (ui->e_password->text().isEmpty() == false) {
                if (checkConnection() == true) auth();
                else ui->statusbar->showMessage("cannot connect.");
            }
            else ui->statusbar->showMessage("enter the password.");
        }
        else ui->statusbar->showMessage("enter the login.");
    }
    else ui->statusbar->showMessage("enter the IP address.");
}

void Client::onRegister_2Clicked()
{
    if (ui->e_newLogin->text().isEmpty() == false) {
        if (ui->e_newPassword->text().length() < 6 || ui->e_newPassword->text().length() > 32) {
            ui->statusbar->showMessage("the password should be 6 to 32 symbols long.");
        }
        else if (ui->e_newPassword->text() == ui->e_repPassword->text()) {
            createNewUser();
        }
        else {
            ui->statusbar->showMessage("passwords don't match.");
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

void Client::refreshUsersList(QByteArray data) {
    activeUsers.clear();
    data.remove(0, QString("s:::u|").length());
    for (int i = 0; data.length() > 0; i++) {
        QString user = data.left(data.indexOf('\n'));
        activeUsers.insert(i, user);

        data.remove(0, data.indexOf('\n')+1);
    }
    model->setStringList(activeUsers);
}

void Client::leaveChatroom()
{
    if (ui->stackedWidget->currentIndex() == 2) {
        openLogInPage();
        ui->te_message->clear();
        ui->te_chat->clear();
    }
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        abortConnection();
    }
}

void Client::sendMessage()
{
    QString message = encript(ui->te_message->toPlainText().toUtf8());

    if(message.length() > 0) {
        m_socket->write(QString("c:::m|" + message).toUtf8());
        ui->te_message->clear();
    }
}

void Client::createNewUser()
{
    if (checkConnection() == true) {
        m_socket->write(QString("c:::r|" + ui->e_newLogin->text()+'\t'+ui->e_newPassword->text()).toUtf8());
    }
    else {
        ui->statusbar->showMessage("cannot connect. try again later.");
    }
}

void Client::auth()
{
    m_socket->write(QString("c:::l|" + ui->e_login->text()+'\t'+ui->e_password->text()).toUtf8());
}

Client::~Client()
{
    delete ui;
}
