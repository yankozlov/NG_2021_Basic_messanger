#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QtSql>
#include <QCryptographicHash>
#include <QRandomGenerator64>
#include <QThread>
#include <QDir>
#include <QDebug>

#include "worker.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();

protected:
    void incomingConnection(qintptr handle);

private slots:
    void droppedConnection();

    void messageReceived(QByteArray msg);
    void addUser(QByteArray login, QByteArray password);
    void auth(QByteArray login, QByteArray password);

    void setDatabase();
    QByteArray getHash(QByteArray password, QByteArray salt);

    bool checkLogin(QByteArray login);
    void refreshUsersList(QString user, QChar action);
    void sendUsersList(Worker *client);

private:
    const int maxMessageLength = 3000;

    QVector <Worker *> m_clients;
    QSqlDatabase database;
    QCryptographicHash *hasher;
};

#endif // SERVER_H
