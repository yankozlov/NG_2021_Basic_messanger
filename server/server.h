#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QCryptographicHash>
#include <QRandomGenerator64>
#include <QThread>
#include <QDir>
#include <QTime>
#include <QDebug>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();

protected:
    void incomingConnection(qintptr handle);

private slots:
    void readyRead();
    void droppedConnection();
    bool addUser(QTcpSocket *client, QByteArray dataset);
    void messageReceived(QTcpSocket *client, QByteArray msg);

    void setDatabase();
    QByteArray getHash(QByteArray password, QByteArray salt);
    bool compHashes(QByteArray login, QByteArray password);

    bool checkLogin(QByteArray login);
    void auth(QTcpSocket *client, QByteArray dataset);
    void refreshUsersList();

private:
    void serverLog(QString msg) { qDebug() << QTime::currentTime().toString() + "|[SERVER]:" + msg; }
    void serverErr(QString msg) { qDebug() << QTime::currentTime().toString() + "|[ERROR]: " + msg; }


    const int maxMessageLength = 3000;

    QVector <QTcpSocket *> m_clients;
    //QMap <QTcpSocket *, MessageThread> m_threads;
    QMap <QTcpSocket *, QString> m_activeUsers;
    QSqlDatabase database;
    QCryptographicHash *hasher;
};

#endif // SERVER_H
