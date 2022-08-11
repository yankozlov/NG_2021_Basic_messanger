#include "worker.h"
#include "logging.h"

Worker::Worker(qintptr ID)
{
    socketDescriptor = ID;
    socket = nullptr;
}

void Worker::setup()
{
    threadLog("thread started", this->thread());

    socket = new QTcpSocket();

    if (socket->setSocketDescriptor(socketDescriptor) == false) {
        qDebug() << socket->errorString();
        delete socket;
        emit finished();
        return;
    }
    address = socket->peerAddress().toString() + ":" + QString::number(socket->peerPort());

    connect(socket, &QTcpSocket::readyRead, this, &Worker::readyRead, Qt::DirectConnection);
    connect(socket, &QTcpSocket::disconnected, this, &Worker::disconnected);

    threadLog(address, this->thread());
    serverLog("just connected!", address);
}

void Worker::write(QByteArray msg = "")
{
    threadLog("sending...", this->thread());
    socket->write(msg);
    threadLog("done.", this->thread());
}

void Worker::readyRead()
{
    QByteArray data = socket->readAll();

    serverLog(data, address);

    if(data.indexOf("c:::m|") == 0) {
        data.remove(0, QString("c:::m|").length());

        emit messageReceived(data);
    }
    else if(data.indexOf("c:::r|") == 0) {
        data.remove(0, QString("c:::r|").length());
        int endOfLogin = data.indexOf('\t');

        QByteArray login = data.left(endOfLogin);
        QByteArray password = data.remove(0, endOfLogin+1);

        emit addUser(login, password);
    }
    else if(data.indexOf("c:::l|") == 0) {
        data.remove(0, QString("c:::l|").length());
        int endOfLogin = data.indexOf('\t');

        QByteArray login = data.left(endOfLogin);
        QByteArray password = data.remove(0, endOfLogin+1);

        emit auth(login, password);
    }
    else serverErr("Unknown protocol.");
}

void Worker::disconnected()
{
    threadLog("thread finished", this->thread());

    socket->deleteLater();
    emit finished();
}
