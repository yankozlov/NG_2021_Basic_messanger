#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QDir>
#include <QDebug>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();

private slots:
    void readyRead();
    void droppedConnection();
    bool addUser(QTcpSocket *client, QByteArray dataset);
    void messageReceived(QTcpSocket *client, QByteArray msg);

    void setDatabase();

    bool checkLogin(QByteArray login);
    void auth(QTcpSocket *client, QByteArray dataset);
    void refreshUsersList();

protected:
    void incomingConnection(qintptr handle);

private:
    void serverLog(QString msg) { qDebug() << "[SERVER]:" << msg; }
    void serverErr(QString msg) { qDebug() << "[ERROR]:" << msg; }

    QVector <QTcpSocket *> m_clients;
    QMap <QTcpSocket *, QString> m_activeUsers;
    QSqlDatabase database;
};

#endif // SERVER_H
