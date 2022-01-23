#include "server.h"

Server::Server()
{

}

void Server::incomingConnection(qintptr handle)
{
    QTcpSocket *client = new QTcpSocket();
    client->setSocketDescriptor(handle);

    m_clients.append(client);
    log("New client connected! [" + client->localAddress().toString() + "]");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

void Server::readyRead()
{
    QTcpSocket *client = (QTcpSocket *)sender();

    QByteArray data = client->readAll();
    if(data.indexOf("c:::m|") == 0) { messageReceived(client, data); }
    if(data.indexOf("c:::r|") == 0) { checkLogin(client, data); }
    if(data.indexOf("c:::l|") == 0) { auth(client, data); }

    log(data);

    err("Command can't be parsed.");
}

void Server::droppedConnection()
{
    QTcpSocket *client = (QTcpSocket *)sender();
    m_clients.remove(m_clients.indexOf(client));
    log(client->localAddress().toString() + " dropped the connection!");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

void Server::messageReceived(QTcpSocket *client, QByteArray msg)
{
    msg.remove(0, QByteArray("c:::m|").length());
    QByteArray data = QString(client->localAddress().toString() + ": " + msg).toUtf8();

    for (QTcpSocket *socket : m_clients) {
        socket->write(data);
    }
}

bool Server::checkLogin(QTcpSocket *client, QByteArray login)
{
    login.remove(0, QByteArray("c:::r|").length());
    qDebug() << "loginChecked";
    return true;
}

bool Server::auth(QTcpSocket *client, QByteArray dataset)
{
    dataset.remove(0, QString("c:::l|").length());
    int endOfLogin = dataset.indexOf('\0');

    QByteArray login = dataset.remove(endOfLogin, dataset.length());

    QByteArray password = dataset.remove(0, endOfLogin+1);

    return true;
}
