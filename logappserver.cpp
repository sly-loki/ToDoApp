#include "logappserver.h"
#include "core.h"
#include "network_def.h"

void LogAppServer::readData()
{

}

LogAppServer::LogAppServer(QObject *parent)
    : QObject(parent)
{
    connect(&socket, SIGNAL(readyRead()), this, SLOT(readData()));
}

bool LogAppServer::connectToServer()
{
    socket.abort();
    socket.connectToHost("localhost", DEBUG_PORT);
}

bool LogAppServer::ping()
{

}

bool LogAppServer::SaveItem(LogItem *item)
{

}

bool LogAppServer::SaveTree(LogItem *root)
{

}

