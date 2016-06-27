#include "server.h"

#include <memory>
#include <functional>
#include <cassert>

TodoServer::TodoServer()
    : clientConnection(nullptr)
    , control(nullptr)
{
    connect(&server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

void TodoServer::start()
{
    if (!server.listen(QHostAddress::Any, DEBUG_PORT)) {
        qDebug() << "server listen error";
        return;
    }
}

void TodoServer::sendPacket(NetworkHeader *header, const void *data)
{
    assert(header);
    if (header->dataSize)
        assert(data != nullptr);

    qDebug() << "send package";

    clientConnection->write((char *)header, sizeof(NetworkHeader));
    if (header->dataSize)
        clientConnection->write((char *)data, header->dataSize);
}

void TodoServer::readPacket()
{

}

void TodoServer::incomingMessage()
{
    if (!control) {
        qDebug() << "LogControl not set";
        return;
    }
    NetworkHeader packet;
    while (true) {
    if (clientConnection->read((char *)(&packet), sizeof(packet)) != sizeof(packet)) {
        qDebug() << "read error";
        return;
    }
    char * text = nullptr;
    if (packet.dataSize) {
        text = new char[packet.dataSize+1];
        size_t readed = clientConnection->read((char *)text, packet.dataSize);
        if (readed != packet.dataSize) {
            qDebug() << "data read error";
            throw "bad data";
        }
    }
    switch (packet.type) {
    case PT_ITEM_CREATED:
    {
        qDebug() << "item created message";
        qDebug() << "id : " << packet.itemId;
        qDebug() << "text: " << text;

        CreateItemData data;
        data.parentId = packet.parentId;
        data.text = "";
        data.prevItemId = 0;

        emit createItem(data);
    }
        break;
    case PT_GET_ITEM:
    {
        qDebug() << "get item: " << packet.itemId;
        LogItem *item = control->findItemById(packet.itemId);
        if (item) {
            packet.dataSize = item->getText().size();
            packet.parentId = (item->getParent())?item->getParent()->getId():0;
            sendPacket(&packet, item->getText().toStdString().c_str());
        } else {
            // error processing here
        }
    }
        break;
    case PT_GET_ALL_ITEMS:
    {
        qDebug() << "request for all item";
        std::vector<uint64_t> ids;
        LogItem *root = control->getRootItem();
        std::function<void (LogItem *, std::vector<uint64_t> &)> f = [&f](LogItem *item, std::vector<uint64_t> &ids){
            ids.push_back(item->getId());
            LogItem *child = item->getChild();
            while (child) {
                f(child, ids);
                child = child->getNext();
            }
        };
        f(root, ids);
        packet.dataSize = ids.size() * sizeof(uint64_t);
        sendPacket(&packet, (const void *)ids.data());
    }
        break;
    case PT_GET_CHILDREN:
    {
        qDebug() << "request for children: " << packet.itemId;
        std::vector<uint64_t> ids;
        LogItem *parent = control->findItemById(packet.itemId);
        if (!parent) {
            qDebug() << "error: no such item";
            packet.dataSize = 0;
            sendPacket(&packet, nullptr);
            break;
        }
        LogItem *child = parent->getChild();
        while(child) {
            ids.push_back(child->getId());
            child = child->getNext();
        }
        packet.dataSize = ids.size() * sizeof(uint64_t);
        sendPacket(&packet, (const void *)ids.data());
    }
        break;
    default:
        qDebug() << "error here";
        break;
    }
    }
}

void TodoServer::onNewConnection()
{
    if (clientConnection)
        delete clientConnection;
    clientConnection = server.nextPendingConnection();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingMessage()));
    qDebug() << "new connection established";
}
