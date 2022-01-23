#include <QCoreApplication>
#include <server.h>

void log(QString msg) { qDebug() << "[CORE]: " + msg; }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *server = new Server();
    bool ok = server->listen(QHostAddress::Any, 8093);

    if (ok) {
        log(("Server started at " + QString::number(8093)).toUtf8());
    }
    else {
        log("Server crashed!");
        log(server->errorString().toUtf8());
    }

    return a.exec();
}
