#include "server.h"

#include <memory>
#include <functional>
#include <cassert>

#include "core.h"

void TodoServer::sendDocumentList(NetworkHeader *header, QTcpSocket *connection)
{
    std::vector<DocumentDescriptor> docDescs;

    for (int i = 0; i < docs.size(); i++) {
        DocumentDescriptor desc;
        desc.id = i;
        memset(desc.name, 0, DOCUMENT_NAME_MAX_LENGHT);
        memcpy(desc.name, "one", sizeof("one"));
        docDescs.push_back(desc);
    }

    NetworkHeader response(*header);
    response.dataSize = docDescs.size()*sizeof(DocumentDescriptor);
    sendPacket(&response, docDescs.data());
}

TodoServer::TodoServer(const std::vector<LogControl *> docs)
    : clientConnection(nullptr)
    , docs(docs)
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

void TodoServer::sendItem(LogItem *item)
{
    NetworkHeader packet;
    qDebug() << "send item";
    packet.type = PT_GET_ITEM;
    packet.itemId = item->getId();
    packet.dataSize = item->getText().size();
    packet.parentId = (item->getParent())?item->getParent()->getId():0;
    sendPacket(&packet, item->getText().toStdString().c_str());
}

void TodoServer::sendChildrenIds(uint64_t itemId, std::vector<uint64_t> &ids)
{
    NetworkHeader packet;
    qDebug() << "send ids" << ids.size();
    packet.type = PT_GET_CHILDREN;
    packet.itemId = itemId;
    packet.dataSize = ids.size() * sizeof(uint64_t);
    sendPacket(&packet, (const void *)ids.data());
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
    case PT_GET_DOC_LIST:
    {
        qDebug() << "get doc list";
        sendDocumentList(&packet, clientConnection);
    }
        break;
    case PT_GET_ITEM:
    {
        qDebug() << "get item: " << packet.itemId;
        emit itemRequested(packet.itemId);
    }
        break;
    case PT_GET_ALL_ITEMS:
    {
//        qDebug() << "request for all item";
//        std::vector<uint64_t> ids;
//        LogItem *root = control->getRootItem();
//        std::function<void (LogItem *, std::vector<uint64_t> &)> f = [&f](LogItem *item, std::vector<uint64_t> &ids){
//            ids.push_back(item->getId());
//            LogItem *child = item->getChild();
//            while (child) {
//                f(child, ids);
//                child = child->getNext();
//            }
//        };
//        f(root, ids);
//        packet.dataSize = ids.size() * sizeof(uint64_t);
//        sendPacket(&packet, (const void *)ids.data());
    }
        break;
    case PT_GET_CHILDREN:
    {
        qDebug() << "request for children: " << packet.itemId;
        emit childrenIdsRequested(packet.itemId);
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
