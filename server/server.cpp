#include "server.h"

#include <memory>
#include <functional>
#include <cassert>

#include "core.h"

void TodoServer::sendDocumentList(NetworkHeader *header, QTcpSocket *connection)
{
    std::vector<DocumentDescriptor> docDescs;

    for (auto it: docs) {

        DocumentDescriptor desc;

        QString name = it.second->getName();
        desc.id = it.first;
        memset(desc.name, 0, DOCUMENT_NAME_MAX_LENGHT);
        memcpy(desc.name, name.toStdString().c_str(), name.length());
        docDescs.push_back(desc);
    }

    NetworkHeader response(*header);
    response.dataSize = docDescs.size()*sizeof(DocumentDescriptor);
    sendPacket(&response, docDescs.data());
}

TodoServer::TodoServer(const std::vector<LogControl *> docs)
    : clientConnection(nullptr)
{
    for (int i = 0; i < docs.size(); i++) {
        this->docs[i] = docs[i];
    }
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

void TodoServer::createItem(NetworkHeader *header, const char *data)
{
    LogControl *doc = docs[header->docId];
    if (!doc) {
        qDebug() << "doc not exists";
        return;
    }
    LogItem *parent = doc->findItemById(header->parentId);
    if (!parent) {
        qDebug() << "parent not exists";
        return;
    }
    LogItem *newItem = new LogItem(doc, parent, 0);
    newItem->setText(data);
    parent->addAsChild(newItem);
}

void TodoServer::sendItem(NetworkHeader *header)
{
    NetworkHeader packet;
    LogControl *doc = docs[header->docId];
    if (!doc) {
        qDebug() << "doc not exists";
        return;
    }
    LogItem *item = doc->findItemById(header->itemId);
    if (!item) {
        qDebug() << "item not exists";
        return;
    }
    qDebug() << "send item";
    packet.type = PT_GET_ITEM;
    packet.itemId = item->getId();
    packet.dataSize = item->getText().size();
    packet.parentId = (item->getParent())?item->getParent()->getId():0;
    sendPacket(&packet, item->getText().toStdString().c_str());
}

void TodoServer::sendChildrenIds(NetworkHeader *header)
{
    NetworkHeader packet;

    std::vector<uint64_t> ids;
    LogControl *doc = docs[packet.docId];
    if (!doc) {
        qDebug() << "error: wrong document id!!!";
        return;
    }
    LogItem *parent = doc->findItemById(header->itemId);
    if (!parent) {
        qDebug() << "error: no such item";
    } else {
        LogItem *child = parent->getChild();
        while(child) {
            ids.push_back(child->getId());
            child = child->getNext();
        }
    }

    packet.type = PT_GET_CHILDREN;
//    packet.itemId = itemId;
    packet.dataSize = ids.size() * sizeof(uint64_t);
    sendPacket(header, (const void *)ids.data());
}

void TodoServer::incomingMessage()
{
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
            text[packet.dataSize] = '\0';
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

            createItem(&packet, text);

        }
            break;
        case PT_ITEM_DELETED:
        {
            qDebug() << "item delete message";
        }
            break;
        case PT_GET_DOC_LIST:
        {
            qDebug() << "get doc list";
            sendDocumentList(&packet, clientConnection);
        }
            break;
        case PT_GET_ITEM:
        {
            qDebug() << "get item: " << packet.itemId;
            sendItem(&packet);
        }
            break;
        case PT_GET_ALL_ITEMS:
        {
            qDebug() << "get all items: not supported";
        }
            break;
        case PT_GET_CHILDREN:
        {
            qDebug() << "request for children: " << packet.docId << "." << packet.itemId;
            sendChildrenIds(&packet);
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
