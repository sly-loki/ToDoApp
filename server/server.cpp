#include "server.h"

#include <memory>
#include <functional>
#include <cassert>

#include <QDir>

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

    clientConnection->setPrivateKey(QDir::homePath() + "/.todo/keys/key.pem");
    clientConnection->setLocalCertificate(QDir::homePath() +  "/.todo/keys/cert.pem");
    clientConnection->ignoreSslErrors();
    qDebug() << "socket state: " << clientConnection->state();
    clientConnection->startServerEncryption();
    qDebug() << "new connection established";
}

void TodoServer::sendDocumentList(NetworkHeader *header, QTcpSocket *connection)
{
    std::vector<DocumentDescriptor> docDescs;

    for (auto it: docs) {

        DocumentDescriptor desc;
        ServerDocument *doc = it.second;

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

TodoServer::TodoServer(QString storageFolderName)
    : clientConnection(nullptr)
    , storageDir(storageFolderName)
{
    if (!storageDir.exists())
        storageDir.mkpath(storageFolderName);

    QStringList filters;
    filters.append("*.xml");
    QStringList fileList = storageDir.entryList(filters);
    qDebug() << fileList;

    std::vector<ServerDocument *> docs;
    uint64_t id = 1000;
    for (auto s: fileList) {
        QString fileName = storageDir.path() + QDir::separator() + s;

        DB *db = new XmlDB(fileName);
        ServerDocument *control = new ServerDocument(db, s, id++);
        control->loadData();
        docs.push_back(control);
    }


    for (int i = 0; i < docs.size(); i++) {
        ServerDocument *doc = docs[i];
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

    clientConnection->write((char *)header, sizeof(NetworkHeader));
    if (header->dataSize)
        clientConnection->write((char *)data, header->dataSize);

    clientConnection->waitForBytesWritten(1000);
}

void TodoServer::readPacket()
{

}

void TodoServer::sendResponse(uint64_t requestID, uint16_t *response)
{
    NetworkHeader header;
    header.requestID = requestID;
    header.type = PT_RESPONSE;
    header.dataSize = sizeof(uint16_t);
    sendPacket(&header, response);
}

uint16_t TodoServer::createItem(ItemDescriptor item)
{
    ServerDocument *doc = docs[item.docId];
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

uint16_t TodoServer::removeItem(ItemDescriptor item)
{
    ServerDocument *doc = docs[item.docId];
    if (!doc) {
        qDebug() << "doc not exists";
        return ER_ITEM_ALREADY_EXIST;
    }

    LogItem *i = doc->findItemById(item.id);
    if (!i) {
        qDebug() << "item not exists";
        return ER_ITEM_NOT_EXIST;
    }

    i->detachFromTree();
    return ER_OK;
}

uint16_t TodoServer::setItemText(NetworkHeader *header, QString text)
{
    ServerDocument *doc = docs[header->docId];
    if (!doc)
        return ER_DOC_NOT_EXIST;

    LogItem *item = doc->findItemById(header->itemId);
    if (!item)
        return ER_ITEM_NOT_EXIST;

    item->setText(text);
    return ER_OK;
}

uint16_t TodoServer::moveItem(ItemDescriptor id)
{
    ServerDocument *doc = docs[id.docId];
    if (!doc)
        return ER_DOC_NOT_EXIST;

    LogItem *item = doc->findItemById(id.id);
    if (!item)
        return ER_ITEM_NOT_EXIST;

    LogItem *newParent = doc->findItemById(id.parentId);
    if (!newParent)
        return ER_ITEM_NOT_EXIST;

    LogItem *newPrev = nullptr;
    if (id.prevId != 0)
        newPrev = doc->findItemById(id.prevId);

    item->detachFromTree();

    newParent->addAsChild(item, newPrev);
    return ER_OK;
}

uint16_t TodoServer::setItemDone(ItemDescriptor id)
{
    ServerDocument *doc = docs[id.docId];
    if (!doc)
        return ER_DOC_NOT_EXIST;

    LogItem *item = doc->findItemById(id.id);
    if (!item)
        return ER_ITEM_NOT_EXIST;

    item->setDone(id.done);
    return ER_OK;
}

uint16_t TodoServer::setItemFold(ItemDescriptor id)
{
    ServerDocument *doc = docs[id.docId];
    if (!doc)
        return ER_DOC_NOT_EXIST;

    LogItem *item = doc->findItemById(id.id);
    if (!item)
        return ER_ITEM_NOT_EXIST;

    item->setFolded(id.folded);
}

uint16_t TodoServer::saveDocument(DocumentDescriptor desc)
{
    ServerDocument *doc = docs[desc.id];
    if (!doc)
        return ER_DOC_NOT_EXIST;
    doc->save();
    return ER_OK;
}

uint16_t TodoServer::createDocument(DocumentDescriptor desc)
{
    ServerDocument *newDoc = nullptr;

    QString noteName = QString((const char *)desc.name);
    QString fileName = storageDir.path() + QDir::separator() + noteName + ".xml";
    QFile file(fileName);
    if (file.exists()) {
        qDebug() << "error: file exists";
        return ER_ITEM_ALREADY_EXIST;
    }

    file.open(QIODevice::ReadWrite);
    if (!file.isOpen())
        return ER_INTERNAL_ERROR;
    file.close();

    DB *db = new XmlDB(fileName);
    newDoc = new ServerDocument(db, noteName, ServerDocument::getNextDocId());

    if (newDoc) {
        newDoc->loadData();
    }

    return ER_OK;
}

uint16_t TodoServer::removeDocument(DocumentDescriptor desc)
{
    Q_UNUSED(desc);
    return ER_OK;
}

void TodoServer::sendItem(NetworkHeader *header)
{
    NetworkHeader packet;
    ServerDocument *doc = docs[header->docId];
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

    ItemDescriptor itemd;
    packet.type = PT_GET_ITEM;
    packet.itemId = item->getId();
    packet.requestID = header->requestID;
    packet.dataSize = item->getText().toStdString().size();
    packet.parentId = (item->getParent())?item->getParent()->getId():0;
    sendPacket(&packet, item->getText().toStdString().c_str());
}

void TodoServer::sendChildrenIds(NetworkHeader *header)
{
    qDebug() << "get children with request id: " << header->requestID;
    std::vector<ItemDescriptor> ids;
    ServerDocument *doc = docs[header->docId];
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
            itemd.done = child->isDone()?1:0;
            itemd.folded = child->isFolded()?1:0;
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
        int readed = clientConnection->read((char *)(&packet), sizeof(packet));
        if (readed == 0)
            return;
        if (readed != sizeof(packet)) {
            if (readed == -1 || readed == 0) {
                qDebug() << "ERROR: read error: " << readed;
//                qDebug() << clientConnection->errorString();
//                qDebug() << clientConnection->error();
//                return;
            } else {
                while (readed < sizeof(packet)) {
                    int sub_read = clientConnection->read(((char *)(&packet)) + readed, sizeof(packet) - readed);
                    if (sub_read  == -1) {
//                        qDebug() << "ERROR: sub read error";
//                        return;
                    }
                    readed += sub_read;
                }
            }
        }
        char * text = nullptr;
        if (packet.dataSize) {
            text = new char[packet.dataSize+1];
            int readed = clientConnection->read((char *)text, packet.dataSize);
            text[packet.dataSize] = '\0';
            if (readed != packet.dataSize) {
                if (readed == -1 || readed == 0) {
                    qDebug() << "ERROR: read error";
  //                  return;
                } else {
                    while (readed < sizeof(packet)) {
                        int sub_read = clientConnection->read(((char *)text) + readed, sizeof(packet) - readed);
                        if (sub_read  == -1) {
                            qDebug() << "ERROR: sub read error";
//                            return;
                        }
                        readed += sub_read;
                    }
                }
            }
        }
        switch (packet.type) {
        case PT_ITEM_CREATED:
        {
            qDebug() << "item created message";
            qDebug() << "id : " << packet.itemId;

            uint16_t result = createItem(*(ItemDescriptor *)text);

            sendResponse(packet.requestID, &result);
        }
            break;
        case PT_ITEM_DELETED:
        {
            qDebug() << "item delete message";
            qDebug() << "id : " << packet.itemId;

            uint16_t result = removeItem(*(ItemDescriptor *)text);

            sendResponse(packet.requestID, &result);
        }
            break;
        case PT_ITEM_CHANGED:
        {
            qDebug() << "item change message";
            qDebug() << packet.docId << "." << packet.itemId;
            qDebug() << "text " << packet.dataSize << " : " << text;

            uint16_t result = setItemText(&packet, QString(text));

            sendResponse(packet.requestID, &result);
        }
            break;
        case PT_ITEM_MOVED:
        {
            qDebug() << "item move message";
            qDebug() << packet.docId << "." << packet.itemId;

            uint16_t result = moveItem(*(ItemDescriptor *)text);

            sendResponse(packet.requestID, &result);
        }
            break;
        case PT_ITEM_DONE_CHANGED:
        {
            qDebug() << "item done message";

            uint16_t result = moveItem(*(ItemDescriptor *)text);

            sendResponse(packet.requestID, &result);
        }
            break;
        case PT_ITEM_FOLD_CHANGED:
        {

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
        case PT_DOC_CREATE:
        {
            qDebug() << "create document request: ";
        }
            break;
        case PT_DOC_SAVE:
        {
            qDebug() << "save doc request: ";
            saveDocument(*(DocumentDescriptor *)text);
        }
            break;
        default:
            qDebug() << "error here: " << packet.type;
            break;
        }
        delete[] text;
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
