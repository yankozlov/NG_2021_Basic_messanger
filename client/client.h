#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTime>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void connected();
    void received();

    void openLogInPage();
    void openRegistrationPage();
    void openChatroomPage();

    bool checkConnection();
    void abortConnection();

    void onRegisterClicked();
    void onLogInClicked();
    void onRegister_2Clicked();
    void onCancelClicked();

    void refreshUsersList(QByteArray data);
    void messageLimiter();
    void sendMessage();
    void leaveChatroom();

    void createNewUser();
    void auth();

private:
    Ui::Client *ui;
    QTcpSocket *m_socket;

    const int maxMessageLength = 3000;

    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
};
#endif // CLIENT_H
