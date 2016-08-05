#include "server.h"

#include <memory>
#include <functional>
#include <cassert>

#include "core.h"

void TodoServer::incomingConnection(qintptr handle)
{
    if (clientConnection)
        delete clientConnection;
    clientConnection = new QSslSocket();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingMessage()));
    connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(clientConnection, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSocketSslError()));
    connect(clientConnection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

//    clientConnection->setProtocol(QSsl::SslV3);

    if (!clientConnection->setSocketDescriptor(handle))
        qDebug() << "error: set descriptor failed";

    clientConnection->setPrivateKey("/home/andrei/.todo/keys/key.pem");
    clientConnection->setLocalCertificate("/home/andrei/.todo/keys/cert.pem");
    qDebug() << "socket state: " << clientConnection->state();
    clientConnection->startServerEncryption();
    qDebug() << "new connection established";
}

void TodoServer::sendDocumentList(NetworkHeader *header, QTcpSocket *connection)
{
    std::vector<DocumentDescriptor> docDescs;

    for (auto it: docs) {

        DocumentDescriptor desc;
        LogControl *doc = it.second;

        QString name = doc->getName();
        desc.id = doc->getId();
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
        LogControl *doc = docs[i];
        this->docs[doc->getId()] = doc;
        qDebug() << "loaded doc: " << doc->getId();
    }
//    connect(&server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

void TodoServer::start()
{
    if (!listen(QHostAddress::Any, DEBUG_PORT)) {
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

uint16_t TodoServer::createItem(ItemDescriptor item)
{
    LogControl *doc = docs[item.docId];
    if (!doc) {
        qDebug() << "doc not exists";
        return ER_ITEM_ALREADY_EXIST;
    }
    LogItem *parent = doc->findItemById(item.parentId);
    if (!parent) {
        qDebug() << "parent not exists";
        return ER_PARENT_NOT_EXIST;
    }
    LogItem *newItem = new LogItem(doc, parent, item.id);
    LogItem *prev = item.prevId?doc->findItemById(item.prevId):nullptr;
    parent->addAsChild(newItem, prev);
    return ER_OK;
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
    packet.requestID = header->requestID;
    packet.dataSize = item->getText().size();
    packet.parentId = (item->getParent())?item->getParent()->getId():0;
    sendPacket(&packet, item->getText().toStdString().c_str());
}

void TodoServer::sendChildrenIds(NetworkHeader *header)
{
    qDebug() << "get children with request id: " << header->requestID;
    std::vector<ItemDescriptor> ids;
    LogControl *doc = docs[header->docId];
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
            ItemDescriptor itemd;
            itemd.id = child->getId();
            itemd.docId = header->docId;
            itemd.parentId = parent->getId();
            itemd.prevId = (child->getPrev())?child->getPrev()->getId():0;
            ids.push_back(itemd);
            child = child->getNext();
        }
    }

//    packet.type = PT_GET_CHILDREN;
//    packet.itemId = itemId;
    header->dataSize = ids.size() * sizeof(ItemDescriptor);
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

            uint16_t result = createItem(*(ItemDescriptor *)text);
            NetworkHeader header;
            header.requestID = packet.requestID;
            header.type = PT_RESPONSE;
            header.dataSize = sizeof(uint16_t);
            sendPacket(&header, &result);
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
//    if (clientConnection)
//        delete clientConnection;
//    QTcpSocket *soc = server.nextPendingConnection();
//    clientConnection = new QSslSocket();
//    clientConnection->setSocketDescriptor(soc->socketDescriptor());
//    clientConnection->startServerEncryption();
//    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingMessage()));
    //    qDebug() << "new connection established";
}

void TodoServer::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "socket error: " << error;
    qDebug() << clientConnection->errorString();
}

void TodoServer::onSocketSslError()
{
    qDebug() << "socket ssl error";
}

void TodoServer::onSocketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "socket state changed: " << state;
}
