#include "client.h"
#include "ui_client.h"
#include "encription.h"

/*              TODO
 * implement personal chat
 * implement custom encripton
 * encript auth and register data
*/

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    m_socket = new QTcpSocket();

    ui->te_message->installEventFilter(this);

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

    connect(ui->te_message, &QTextEdit::textChanged, this, &Client::messageLimiter);
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
        QString time_str = "[" + QTime::currentTime().toString().remove(5, 3) + "]";

        receivedData.remove(0, QString("s:::m|").length());

        QString senderUsername = receivedData.left(receivedData.indexOf(':'));
        receivedData.remove(0, senderUsername.length()+2);

        receivedData = (time_str + senderUsername + ": " + decript(QString(receivedData))).toUtf8();

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

    if (ui->e_IP->text().isEmpty() == false)
        ui->e_login->setFocus();

    ui->stackedWidget->setCurrentIndex(0);
}

void Client::openRegistrationPage()
{
    ui->e_newLogin->clear();
    ui->e_newPassword->clear();
    ui->e_repPassword->clear();

    ui->e_newLogin->setFocus();

    ui->stackedWidget->setCurrentIndex(1);
}

void Client::openChatroomPage()
{
    ui->te_message->setFocus();
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
        else ui->statusbar->showMessage("passwords don't match.");
    }
    else ui->statusbar->showMessage("enter the login.");
}

void Client::onCancelClicked()
{
    openLogInPage();
    abortConnection();
}

void Client::refreshUsersList(QByteArray data) {
    ui->lw_usersOnline->clear();
    data.remove(0, QString("s:::u|").length());

    const QByteArrayList users = data.split('\t');
    for (QString user : users) {
        if (user == ui->e_login->text()) user.append(" (you)");
        ui->lw_usersOnline->addItem(new QListWidgetItem(QIcon(":/user.png"), user));
    }
}

void Client::messageLimiter()
{
    if (ui->te_message->toPlainText().length() > maxMessageLength) {
        QString message = ui->te_message->toPlainText();
        message.truncate(maxMessageLength);
        ui->te_message->setText(message);
    }
    else if (ui->te_message->toPlainText().length() == maxMessageLength) {
        ui->statusbar->showMessage("you've reached the limit of message length: " + QString::number(maxMessageLength) + " symbols");
        ui->te_message->moveCursor(QTextCursor::End);
    }
    else {
        ui->statusbar->clearMessage();
    }
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
    QString message = encript(ui->te_message->toPlainText());

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

void Client::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (ui->stackedWidget->currentIndex() == 1) onCancelClicked();
        else if (ui->stackedWidget->currentIndex() == 2) leaveChatroom();
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (ui->stackedWidget->currentIndex() == 0) {
            if (ui->e_IP->hasFocus() && !(ui->e_IP->text().isEmpty())) {
                ui->sb_port->setFocus();
                if (ui->sb_port->value() == 0)
                    ui->sb_port->clear();
            }
            else if (ui->sb_port->hasFocus() && (ui->e_login->text().isEmpty() ||
                                                 ui->e_password->text().isEmpty()))
                ui->e_login->setFocus();
            else if (ui->e_login->hasFocus() && !(ui->e_login->text().isEmpty()))
                ui->e_password->setFocus();
            else if (ui->b_register->hasFocus())
                onRegisterClicked();
            else
                onLogInClicked();
        }
        else if (ui->stackedWidget->currentIndex() == 1) {
            if (ui->e_newLogin->hasFocus() && !(ui->e_newLogin->text().isEmpty()))
                ui->e_newPassword->setFocus();
            else if (ui->e_newPassword->hasFocus() && !(ui->e_newPassword->text().isEmpty()))
                ui->e_repPassword->setFocus();
            else if (ui->b_cancel->hasFocus())
                onCancelClicked();
            else
                onRegister_2Clicked();
        }
        else if (ui->stackedWidget->currentIndex() == 2) {
            if (ui->b_send->hasFocus())
                sendMessage();
            else if (ui->b_leave->hasFocus())
                leaveChatroom();
        }
    }
}

bool Client::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ((key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return) && key->modifiers() != Qt::SHIFT) {
            if (ui->te_message->hasFocus()) Client::sendMessage();

            return true;
        }
        else return QObject::eventFilter(obj, event);
    }
    else return QObject::eventFilter(obj, event);
}

Client::~Client()
{
    delete ui;
}
