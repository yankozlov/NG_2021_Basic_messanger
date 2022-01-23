#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QSpinBox>
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
    void openLogInPage();
    void openRegistrationPage();
    void openChatroomPage();

    bool checkConnection();
    void abortConnection();

    void onRegisterClicked();
    void onLogInClicked();
    void onRegister_2Clicked();
    void onCancelClicked();
    void sendMessage();
    void leaveChatroom();

    bool isLoginFree();
    void createNewUser();
    bool auth();

private:
    Ui::Client *ui;
};
#endif // CLIENT_H
