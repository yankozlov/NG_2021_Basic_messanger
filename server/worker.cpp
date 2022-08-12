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

    QString protocol = data.left(QString("c:::#|").length());
    data.remove(0, QString("c:::#|").length());

    if(protocol == "c:::m|") {
        emit messageReceived(data);
    }
    else if(protocol == "c:::r|") {
        QByteArrayList creds = data.split('\t');
        emit addUser(creds[0], creds[1]);
    }
    else if(protocol == "c:::l|") {
        QByteArrayList creds = data.split('\t');
        emit auth(creds[0], creds[1]);
    }
    else serverErr("Unknown protocol.");
}

void Worker::disconnected()
{
    threadLog("thread finished", this->thread());

    socket->deleteLater();
    emit finished();
}
