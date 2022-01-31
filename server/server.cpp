#include "server.h"

Server::Server()
{
    QDir path = QDir::currentPath();
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(path.filePath("users.db"));

    if(database.open()) {
        if (database.tables().isEmpty()) {
            setDatabase();
        }
        QSqlQuery query;
        query.exec("DELETE FROM USERS WHERE NICKNAME IS NULL;");
    }
}

void Server::incomingConnection(qintptr handle)
{
    QTcpSocket *client = new QTcpSocket();
    client->setSocketDescriptor(handle);

    m_clients.append(client);
    serverLog(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + " just connected!");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

void Server::readyRead()
{
    QTcpSocket *client = (QTcpSocket *)sender();
    QByteArray data = client->readAll();

//    if (data.indexOf("c:::m|") != 0) {
//        serverLog(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "|" + data.left(6));
//    }

    if(data.indexOf("c:::m|") == 0) { messageReceived(client, data); }
    else if(data.indexOf("c:::r|") == 0) { addUser(client, data); }
    else if(data.indexOf("c:::l|") == 0) { auth(client, data); }
    else serverErr("Unknown protocol.");
}

void Server::droppedConnection()
{
    QTcpSocket *client = (QTcpSocket *)sender();
    if (m_clients.contains(client)) {
        m_clients.removeOne(client);
        m_activeUsers.remove(client);
        refreshUsersList();
    }
    serverLog(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + " dropped the connection!");

    connect(client, &QTcpSocket::disconnected, this, &Server::droppedConnection);
    connect(client, &QTcpSocket::readyRead, this, &Server::readyRead);
}

bool Server::addUser(QTcpSocket *client, QByteArray dataset)
{
    dataset.remove(0, QString("c:::r|").length());
    int endOfLogin = dataset.indexOf('\t');

    QByteArray login = dataset.left(endOfLogin);

    QByteArray password = dataset.remove(0, endOfLogin+1);

    if(checkLogin(login) == true) {
        QSqlQuery query;
        query.exec(QString("INSERT INTO USERS (NICKNAME, PASSWORD) VALUES('%1', '%2');")
                            .arg(QString(login), QString(password)));

        serverLog("Added new user: [" + login + "]");

        client->write("s:::r|Permitted.");
        return true;
    }
    else {
        serverErr("Login '" + login + "' is already taken.");
        client->write("s:::r|Forbidden.");
        return false;
    }
}

void Server::messageReceived(QTcpSocket *client, QByteArray msg)
{
    msg.remove(0, QByteArray("c:::m|").length());
    QByteArray data = (m_activeUsers[client] + ": " + QString(msg)).toUtf8();

    serverLog(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "|" + data);

    for (QTcpSocket *socket : m_clients) {
        socket->write("s:::m|"+data);
    }
}

void Server::setDatabase()
{
/*      if database is being deleted while server works
   if(*.....*) {
        database.close();
        Server();
        return;
    }
*/
    QSqlQuery query;
    query.exec("CREATE TABLE USERS "
               "(NICKNAME VARCHAR(30), "
               "PASSWORD VARCHAR(32))");
}

bool Server::checkLogin(QByteArray login)
{
    QSqlQuery query;
    query.exec(QString("SELECT * FROM USERS WHERE NICKNAME = '%1';").arg(QString(login)));

    query.next();

    return !query.isValid();
}

void Server::auth(QTcpSocket *client, QByteArray dataset)
{
    dataset.remove(0, QString("c:::l|").length());
    int endOfLogin = dataset.indexOf('\t');

    QByteArray login = dataset.left(endOfLogin);

    QByteArray password = dataset.remove(0, endOfLogin+1);

    QSqlQuery query;
    query.exec(QString("SELECT * FROM USERS WHERE NICKNAME = '%1' AND PASSWORD = '%2';")
                        .arg(QString(login), QString(password)));

    query.next();

    if (query.isValid()) {
        serverLog(QString(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "|"
                          + login + "| Authorisation successful!"));

        client->write("s:::l|Permitted.");

        m_activeUsers.insert(client, login);

        refreshUsersList();
    }
    else {
        serverErr(QString(client->peerAddress().toString() + ":" + QString::number(client->peerPort())
                          + "| Authorisation declined."));

        client->write("s:::l|Forbidden.");
    }
}

void Server::refreshUsersList()
{
    QList<QString> usersList;

    foreach (QString value, m_activeUsers) {
        usersList.append(value);
    }

    usersList.removeDuplicates();

    QString usersString = "s:::u|";
    for (int i = 0; i < usersList.size(); i++) {
        usersString.append(usersList.at(i)).append('\n');
    }

    serverLog(usersString);
    for (QTcpSocket *socket : m_clients) {
        socket->write(usersString.toUtf8());
    }
}
