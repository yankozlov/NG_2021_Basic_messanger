#include "logging.h"

#define TIME QTime::currentTime().toString()

void coreLog(QString msg)
{
    qDebug().noquote() << TIME + "|[CORE]:  " << msg;
}

void coreErr(QString msg)
{
    qDebug().noquote() << TIME + "|[ERROR]: " << msg;
}

void serverLog(QString msg)
{
    qDebug().noquote() << TIME + "|[SERVER]:" << msg;
}

void serverLog(QString msg, QString address)
{
    qDebug().noquote() << TIME + "|[SERVER]:" << address + "|" + msg;
}

void serverErr(QString msg)
{
    qDebug().noquote() << TIME + "|[ERROR]: " << msg;
}

void serverErr(QString msg, QString address)
{
    qDebug().noquote() << TIME + "|[ERROR]: " << address + "|" + msg;
}

void hashLog(QString msg) {
    qDebug().noquote() << "        |[HASH]:  " << msg;
}

void dbHashLog(QString msg) {
    qDebug().noquote() << "        |[DBHASH]:" << msg;
}

void threadLog(QString msg, QThread* thread)
{
    qDebug().noquote() << "        |[THREAD]:" << thread << "|" + msg;
}
