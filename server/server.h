#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
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

    bool checkLogin(QTcpSocket *client, QByteArray login);
    bool auth(QTcpSocket *client, QByteArray dataset);

protected:
    void incomingConnection(qintptr handle);

private:
    void log(QString msg) { qDebug() << "[SERVER]:" << msg; }
    void err(QString msg) { qDebug() << "[ERROR]:" << msg; }

    QVector <QTcpSocket *> m_clients;

};

#endif // SERVER_H
