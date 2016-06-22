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
    header->requestID = request_id;

    request_id++;

    socket.write((char *)header, sizeof(NetworkHeader));
    if (header->dataSize)
        socket.write((char *)data, header->dataSize);
}

void LogAppServer::readData()
{
    NetworkHeader header;
    while (true) {
    int readed = socket.read((char *)&header, sizeof(NetworkHeader));

    if (readed != sizeof(NetworkHeader))
        return;

    switch (header.type) {
    case PT_GET_ALL_ITEMS: {
        qDebug() << "get all items response";
        size_t itemCount = header.dataSize/sizeof(uint64_t);
        uint64_t *ids = new uint64_t[itemCount];
        size_t readed = socket.read((char *)ids, header.dataSize);
        if (readed != header.dataSize) {
            qDebug() << "ERROR: ";
            delete[] ids;  //fix
            return;
        }
        qDebug() << "readed " << itemCount << " items";
        for (int i = 0; i < itemCount; i++) {
            qDebug() << ids[i];
        }
        emit itemListReceived(ids, itemCount);

        delete[] ids;
    }
        break;
    case PT_GET_ITEM: {
        ServerItemData itemData;

        char *text = new char[header.dataSize];
        size_t readed = socket.read(text, header.dataSize);
        if (readed != header.dataSize) {
            qDebug() << "ERROR: ";
            delete[] text;  //fix
            return;
        }

        itemData.itemId = header.itemId;
        itemData.parentId = header.parentId;
        itemData.text = QString(text);
        emit itemReceived(itemData);

        delete[] text;

    }
        break;
    default:
        break;
    }
    }
}

LogAppServer::LogAppServer(QObject *parent)
    : QObject(parent)
    , request_id(0)
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

bool LogAppServer::saveItem(LogItem *item)
{

}

bool LogAppServer::saveTree(LogItem *root)
{

}

void LogAppServer::getItemList()
{
    NetworkHeader header;
    header.type = PT_GET_ALL_ITEMS;
    header.dataSize = 0;
    sendPacket(&header, nullptr);
}

void LogAppServer::getItemData(uint64_t id)
{
    NetworkHeader header;
    header.itemId = id;
    header.dataSize = 0;
    header.type = PT_GET_ITEM;

    sendPacket(&header, nullptr);
}

void LogAppServer::addItem(LogItem *item)
{
    NetworkHeader header;
    header.itemId = item->getId();
    header.parentId = item->getParent()->getId();
    header.dataSize = item->getText().length();
    header.type = PT_ITEM_CREATED;

//    sendPacket(&header, item->getText().toStdString().c_str()); //arghhhh
}

void LogAppServer::removeItem(LogItem *item)
{

}

void LogAppServer::sendAction(ServerAction action)
{

}

