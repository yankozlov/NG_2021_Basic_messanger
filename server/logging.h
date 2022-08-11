#ifndef LOGGING_H
#define LOGGING_H

#include <QTime>
#include <QDebug>

void coreLog  (QString msg);
void coreErr  (QString msg);

void serverLog(QString msg, QString address = "");
void serverErr(QString msg, QString address = "");

void hashLog  (QString msg);
void dbHashLog(QString msg);

void threadLog(QString msg, QThread* thread);

#endif // LOGGING_H
