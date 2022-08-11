#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QTcpSocket>
#include <QtNetwork>

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker(qintptr ID);

    QString getAddress() { return address; }
    QString getNickname() { return nickname; }

    void setNickname(QByteArray nickname) { this->nickname = nickname; }

public slots:
    void setup();
    void readyRead();
    void disconnected();

    void write(QByteArray msg);

signals:
    void send(QByteArray msg);

    void messageReceived(QByteArray data);
    void addUser(QByteArray login, QByteArray password);
    void auth(QByteArray login, QByteArray password);

    void finished();

private:
    qintptr socketDescriptor;
    QTcpSocket* socket;
    QString nickname;
    QString address;
};

#endif // WORKER_H
