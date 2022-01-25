#include <QCoreApplication>
#include <server.h>

void coreLog(QString msg) { qDebug() << "[CORE]: " + msg; }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *server = new Server();
    bool ok = server->listen(QHostAddress::Any, 8093);

    if (ok) {
        coreLog(("Server started at " + QString::number(8093)).toUtf8());
    }
    else {
        coreLog("Server crashed!");
        coreLog(server->errorString().toUtf8());
    }

    return a.exec();
}
