#include <QCoreApplication>
#include <server.h>

#include "logging.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *server = new Server();
    bool ok = server->listen(QHostAddress::Any, 8093);

    if (ok) {
        coreLog(QString("Server started at port %1").arg(server->serverPort()));
    }
    else {
        coreLog("Cannot start the server!");
        coreLog(server->errorString());
    }

    return a.exec();
}
