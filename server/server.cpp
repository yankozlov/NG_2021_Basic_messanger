#include "server.h"
#include "logging.h"

/*                              TODO
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
        coreErr("Server might work incorrectly: SQL database doesn't open");
    }
}

void Server::incomingConnection(qintptr handle)
{
    Worker* client = new Worker(handle);
    QThread* thread = new QThread;
    client->moveToThread(thread);

    connect(thread, &QThread::started, client, &Worker::setup);

    connect(client, &Worker::messageReceived, this, &Server::messageReceived);
    connect(client, &Worker::addUser, this, &Server::addUser);
    connect(client, &Worker::auth, this, &Server::auth);

    connect(client, &Worker::send, client, &Worker::write);

    connect(client, &Worker::finished, this, &Server::droppedConnection);
    connect(client, &Worker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    m_clients.append(client);
    thread->start();
}

void Server::droppedConnection()
{
    Worker *client = (Worker *)sender();

    if (m_clients.contains(client)) {
        m_clients.removeOne(client);
        if (client->getNickname() != "")
            refreshUsersList(client->getNickname(), '-');
    }
    serverLog("dropped the connection!", client->getAddress());
    client->deleteLater();
}

void Server::addUser(QByteArray login, QByteArray password)
{
    Worker *client = (Worker*)sender();

    qint64 rand = QRandomGenerator64::global()->generate();
    QByteArray salt = QByteArray((const char*)&rand, sizeof(rand)).toHex(0);

    QByteArray hash = getHash(password, salt);

    if(checkLogin(login) == true) {
        QSqlQuery query;
        query.exec(QString("INSERT INTO USERS (NICKNAME, PASSWORD) VALUES('%1', '%2');")
                            .arg(QString(login), QString(hash)));

        serverLog(QString("Added new user: '%1'").arg(QString(login)));
        emit client->send("s:::r|Permitted.");
    }
    else {
        serverErr(QString("Login '%1' is already taken.").arg(QString(login)));
        emit client->send("s:::r|Forbidden.");
    }
}

void Server::messageReceived(QByteArray msg)
{
    Worker *client = (Worker*)sender();

    QString buffer = msg;
    buffer.truncate(maxMessageLength);

    msg = QString("s:::m|" + client->getNickname() + ": " + buffer).toUtf8();

    for (Worker *client : m_clients) {
        if (client->getNickname() != "")
            emit client->send(msg);
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
    password.append(salt);

    hasher->reset();
    hasher->addData(password);

    QByteArray hash = hasher->result();
    hash = salt + '$' + hash.toHex(0);

    return hash;
}

bool Server::checkLogin(QByteArray login)
{
    QSqlQuery query;
    query.exec(QString("SELECT * FROM USERS WHERE NICKNAME = '%1';")
               .arg(QString(login)));

    query.next();

    return !query.isValid();
}

void Server::auth(QByteArray login, QByteArray password)
{
    Worker *client = (Worker*)sender();
    bool isSuccessful = false;

    QSqlQuery query;
    query.exec(QString("SELECT * FROM USERS WHERE NICKNAME = '%1';")
               .arg(QString(login)));
    query.next();

    if (query.isValid()) {
        QByteArray dbHash = query.value(1).toByteArray();

        QByteArray salt = dbHash.split('$').at(0);
        QByteArray hash = getHash(password, salt);

        hashLog(hash);
        dbHashLog(dbHash);

        if (dbHash == hash) isSuccessful = true;
    }

    if (isSuccessful) {
        serverLog(QString("%1| Authorisation successful!").arg(QString(login)), client->getAddress());
        emit client->send("s:::l|Permitted.");

        client->setNickname(login);
        refreshUsersList(login, '+');
        sendUsersList(client);
    }
    else {
        serverErr("Authorisation declined.", client->getAddress());
        emit client->send("s:::l|Forbidden.");
    }
}

void Server::refreshUsersList(QString user, QChar action) {
    // action is supposed to be either '+' or '-'
    QByteArray protocol = "s:::u|";

    QString msg = QString(protocol) + action + user;

    QStringList buffer;
    for (Worker *item : m_clients) {
        buffer.append(item->getNickname());
    }

    if ((action == "-" && buffer.count(user) == 0) ||
        (action == "+" && buffer.count(user) == 1)) {

        serverLog(msg);
        for (Worker *client : m_clients) {
            if (client->getNickname() != user) {
                emit client->send(msg.toUtf8());
            }
        }
    }
}

void Server::sendUsersList(Worker *client) {
    QByteArray protocol = "s:::u|#";

    QStringList buffer;
    for (Worker *item : m_clients) {
        QString nickname = item->getNickname();
        if (nickname != "") {
            buffer.append(nickname);
        }
    }

    buffer.removeDuplicates();
    buffer.sort(Qt::CaseInsensitive);

    emit client->send(protocol + buffer.join('\t').toUtf8());
}
