#include "server.h"

Server::Server()
{

}

void Server::incomingConnection(qintptr handle)
{
    QTcpSocket *client = new QTcpSocket();
    client->setSocketDescriptor(handle);

    m_clients.append(client);
    log(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + " just connected!");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

void Server::readyRead()
{
    QTcpSocket *client = (QTcpSocket *)sender();
    QByteArray data = client->readAll();

    log(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "| " + data);

    if(data.indexOf("c:::m|") == 0) { messageReceived(client, data); }
    else if(data.indexOf("c:::r|") == 0) { addUser(client, data); }
    else if(data.indexOf("c:::l|") == 0) { auth(client, data); }
    else err("Unknown protocol.");
}

void Server::droppedConnection()
{
    QTcpSocket *client = (QTcpSocket *)sender();
    m_clients.remove(m_clients.indexOf(client));
    log(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + " dropped the connection!");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

bool Server::addUser(QTcpSocket *client, QByteArray dataset)
{
    dataset.remove(0, QString("c:::r|").length());
    int endOfLogin = dataset.indexOf('\t');

    QByteArray login = dataset.remove(endOfLogin, dataset.length());
    QByteArray password = dataset.remove(0, endOfLogin+1);

    log(dataset);

    if(checkLogin(client, login) == true) {
        //create new entry
        log(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) +
                                                    "| Added new user: [" + login + "]");
        client->write("s:::r|Permitted.");
        return true;
    }
    else {
        err("Login '" + login + "' is already taken.");
        client->write("s:::r|Forbidden.");
        return false;
    }
}

void Server::messageReceived(QTcpSocket *client, QByteArray msg)
{
    msg.remove(0, QByteArray("c:::m|").length());
    QByteArray data = QString(msg).toUtf8();

    for (QTcpSocket *socket : m_clients) {
        socket->write(data);
    }
}

bool Server::checkLogin(QTcpSocket *client, QByteArray login)
{
    //check login...
    qDebug() << "loginChecked";
    return true;
}

bool Server::auth(QTcpSocket *client, QByteArray dataset)
{
    dataset.remove(0, QString("c:::l|").length());
    int endOfLogin = dataset.indexOf('\t');

    QByteArray login = dataset.remove(endOfLogin, dataset.length());

    QByteArray password = dataset.remove(0, endOfLogin+1);

    return true;
}
