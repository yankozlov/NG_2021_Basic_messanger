#include "server.h"

/*                              TODO
 * handle the case of database is being deleted while server works
 * implement the thread sending of messages
 * provide saving of messages
 * decript encripted auth/register data from client
 */

Server::Server()
{
    hasher = new QCryptographicHash(QCryptographicHash::Sha3_256);
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
    else {
        serverErr("Server might work incorrectly: SQL database doesn't open");
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
        //m_threads.remove(client);
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

    qint64 rand = QRandomGenerator64::global()->generate();
    QByteArray salt = QByteArray((const char*)&rand, sizeof(rand)).toHex(0);
    qDebug() << rand << " | " << salt;

    QByteArray hash = getHash(password, salt);
    qDebug() << hash;

    if(checkLogin(login) == true) {
        QSqlQuery query;
        query.exec(QString("INSERT INTO USERS (NICKNAME, PASSWORD) VALUES('%1', '%2');")
                            .arg(QString(login), QString(hash)));

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
    msg.truncate(maxMessageLength);
    QByteArray data = (m_activeUsers[client] + ": " + QString(msg)).toUtf8();

    serverLog(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "|" + data);

    for (QTcpSocket *socket : m_clients) {
        socket->write("s:::m|"+data);
    }
}

void Server::setDatabase()
{
    QSqlQuery query;
    query.exec("CREATE TABLE USERS "
               "(NICKNAME VARCHAR(30), "
               "PASSWORD VARCHAR(32))");
}

QByteArray Server::getHash(QByteArray password, QByteArray salt)
{
    //    return password; // for plain-text password storage

    password.append(salt);

    hasher->reset();
    hasher->addData(password);

    QByteArray hash = hasher->result();
    hash = salt + '$' + hash.toHex(0);

    qDebug() << "HASH  " << hash;
    return hash;
}

bool Server::compHashes(QByteArray login, QByteArray password)
{
    QSqlQuery query;
    query.exec(QString("SELECT * FROM USERS WHERE NICKNAME = '%1';")
                        .arg(QString(login)));
    query.next();

    if (query.isValid()) {
        QByteArray dbHash = query.value(1).toByteArray();
        QByteArray salt = dbHash.split('$').at(0);

        QByteArray hash = getHash(password, salt);
        qDebug() << "DBHASH" << dbHash;

        if (dbHash == hash) return true;
    }
    return false;
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

    if (compHashes(login, password)) {
        serverLog(QString(client->peerAddress().toString() + ":" + QString::number(client->peerPort()) + "|"
                          + login + "| Authorisation successful!"));

        client->write("s:::l|Permitted.");
/*
        MessageThread messageThread;
        messageThread.setSocket(client);
        m_threads.insert(client, messageThread);
*/
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

    serverLog("Active users list" + usersString);
    for (QTcpSocket *socket : m_clients) {
        socket->write(usersString.toUtf8());
    }
}
