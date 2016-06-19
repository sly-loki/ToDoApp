#include "logappserver.h"

#include <cassert>

#include "core.h"
#include "network_def.h"

void LogAppServer::sendPacket(NetworkHeader *header, const void *data)
{
    assert(header);
    if (header->dataSize)
        assert(data != nullptr);

    qDebug() << "send package";

    socket.write((char *)header, sizeof(NetworkHeader));
    if (header->dataSize)
        socket.write((char *)data, header->dataSize);
}

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

void LogAppServer::addItem(LogItem *item)
{
    NetworkHeader header;
    header.itemId = item->getId();
    header.parentId = item->getParent()->getId();
    header.dataSize = item->getText().length();
    header.type = PT_ITEM_CREATED;

    sendPacket(&header, item->getText().toStdString().c_str()); //arghhhh
}

void LogAppServer::removeItem(LogItem *item)
{

}

