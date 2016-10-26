#include "logappserver.h"


#include <cassert>
#include <QtEndian>
#include <QDir>

#include "core.h"

void LogAppServer::sendPacket(NetworkHeader *header, const void *data)
{
    assert(header);
    if (header->dataSize)
        assert(data != nullptr);

    int writed = socket.write((char *)header, sizeof(NetworkHeader));
    if (writed != sizeof(NetworkHeader)) {
        qDebug() << "ERROR: write error: " << writed;
        if (writed == -1)
            qDebug() << socket.errorString();

    }
    if (header->dataSize) {
        writed = socket.write((char *)data, header->dataSize);
        if (writed != header->dataSize) {
            qDebug() << "ERROR: data write error: " << writed << " need: " << header->dataSize;
            if (writed == -1)
                qDebug() << socket.errorString();
        }
    }
    socket.waitForBytesWritten(1000);
}

void LogAppServer::sendPacketSync(NetworkHeader *header, const void *data)
{
    sendPacket(header, data);
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

            //DEPRECATED
        }
            break;
        case PT_GET_ITEM: {
            ServerItemData itemData;
            qDebug() << "get item response: " << header.itemId;
            qDebug() << "text: " << header.dataSize;
            if (header.dataSize) {
                char *text = new char[header.dataSize+1];
                size_t readed = socket.read(text, header.dataSize);
                if (readed != header.dataSize) {
                    qDebug() << "ERROR: ";
                    delete[] text;  //fix
                    return;
                }
                text[header.dataSize] = 0;
                itemData.text = QString(text);
                delete[] text;
            }

            itemData.itemId = header.itemId;
            itemData.parentId = header.parentId;

            auto it = requests.find(header.requestID);
            if (it != requests.end()) {
                RemoteDB *db = (*it).second;
                db->onItemReceived(itemData);
                requests.erase(it);
            } else {
                qDebug() << "error: no db for this request!";
            }
        }
            break;
        case PT_GET_CHILDREN: {
            qDebug() << "get children items response: " << header.requestID;
            size_t itemCount = header.dataSize/sizeof(ItemDescriptor);
            ItemDescriptor *ids = nullptr;

            if (itemCount != 0) {
                ids = new ItemDescriptor[itemCount];
                size_t readed = socket.read((char *)ids, header.dataSize);
                if (readed != header.dataSize) {
                    qDebug() << "ERROR: ";
                    delete[] ids;  //fix
                    return;
                }
                qDebug() << "readed " << itemCount << " items";
//                for (int i = 0; i < itemCount; i++) {
//                    qDebug() << ids[i];
//                }
            } else {
                qDebug() << "no children items";
            }

            auto it = requests.find(header.requestID);
            if (it != requests.end()) {
                RemoteDB *db = (*it).second;
                db->onItemListReceived(header.itemId, ids, itemCount);
                requests.erase(it);
            } else {
                qDebug() << "Error: not db registered for this request";
            }

            delete[] ids;
        }
            break;
        case PT_GET_DOC_LIST: {
            qDebug() << "get doc list";
            std::vector<std::pair<uint64_t, QString>> docs;
            int count = header.dataSize / sizeof(DocumentDescriptor);
            DocumentDescriptor *descs = new DocumentDescriptor[count];
            size_t readed = socket.read((char *)descs, header.dataSize);
            if (readed != header.dataSize) {
                qDebug() << "ERROR: ";
                delete[] descs;  //fix
                return;
            }
            for (int i = 0; i < count; i++) {
                uint64_t id = descs[i].id;
                docs.push_back( {id, QString((char *)(descs[i].name))} );
            }
            emit docListReceived(docs);
        }
            break;
        case PT_RESPONSE: {

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
//    socket.setProtocol(QSsl::P);
    connect(&socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(&socket, SIGNAL(encrypted()), this, SLOT(onConnectionEstablished()));
//    connect(&socket, SIGNAL(connected()), this, SLOT(onConnectionEstablished()));
    connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(onConnectionLost()));
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onConnectionError(QAbstractSocket::SocketError)));
    connect(&socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onConnectionSslError(QList<QSslError>)));
    socket.addCaCertificates(QDir::homePath() + "/.todo/keys/cert.pem");
}

void LogAppServer::connectToServer()
{
//    socket.abort();
    QAbstractSocket::SocketState state = socket.state();
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        qDebug() << "connecting to server";
        socket.connectToHostEncrypted("localhost", DEBUG_PORT);
        status = SS_CONNECTING;
        break;
    case QAbstractSocket::ConnectedState:
        status = SS_CONNECTED;
        break;
    default:
        break;
    }
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

void LogAppServer::getItemData(uint64_t docId, uint64_t id, RemoteDB *db)
{
    NetworkHeader header;

    header.requestID = request_id;
    requests[header.requestID] = db;
    request_id++;

    header.docId = docId;
    header.itemId = id;
    header.dataSize = 0;
    header.type = PT_GET_ITEM;

    sendPacket(&header, nullptr);
}

void LogAppServer::getItemChildren(uint64_t docId, uint64_t id, RemoteDB *db)
{
    NetworkHeader header;
    header.requestID = request_id;
    requests[header.requestID] = db;
    request_id++;

    header.docId = docId;
    header.itemId = id;
    header.dataSize = 0;
    header.type = PT_GET_CHILDREN;
    qDebug() << "get item children request id: " << header.requestID;

    sendPacket(&header, nullptr);
}

void LogAppServer::getDocList()
{
    NetworkHeader header;
    header.itemId = 0;
    header.dataSize = 0;
    header.type = PT_GET_DOC_LIST;

    sendPacket(&header, nullptr);
}

void LogAppServer::addItem(ItemDescriptor item)
{
    NetworkHeader header;
    header.itemId = item.id;
    header.docId = item.docId;

    header.dataSize = sizeof(ItemDescriptor);
    header.type = PT_ITEM_CREATED;

    sendPacket(&header, &item);
}

void LogAppServer::changeItem(ItemDescriptor item, QString text)
{
    NetworkHeader header;
    header.itemId = item.id;
    header.docId = item.docId;

    header.dataSize = text.toStdString().size();
    header.type = PT_ITEM_CHANGED;

    sendPacket(&header, text.toStdString().c_str());
}

void LogAppServer::moveItem(ItemDescriptor item)
{
    NetworkHeader header;
    header.itemId = item.id;
    header.docId = item.docId;

    header.dataSize = sizeof(ItemDescriptor);
    header.type = PT_ITEM_MOVED;

    sendPacket(&header, &item);
}

void LogAppServer::removeItem(ItemDescriptor item)
{
    NetworkHeader header;
    header.itemId = item.id;
    header.docId = item.docId;

    header.dataSize = sizeof(ItemDescriptor);
    header.type = PT_ITEM_DELETED;

    sendPacket(&header, &item);
}

void LogAppServer::sendAction(ServerAction action)
{

}

void LogAppServer::setItemDone(ItemDescriptor id)
{
    NetworkHeader header;

    header.dataSize = sizeof(ItemDescriptor);
    header.type = PT_ITEM_DONE_CHANGED;

    sendPacket(&header, &id);
}

void LogAppServer::setItemFolded(ItemDescriptor id)
{
    NetworkHeader header;

    header.dataSize = sizeof(ItemDescriptor);
    header.type = PT_ITEM_FOLD_CHANGED;

    sendPacket(&header, &id);
}

void LogAppServer::createDocument(DocumentDescriptor docDesc)
{
    NetworkHeader header;

    header.dataSize = sizeof(DocumentDescriptor);
    header.type = PT_DOC_CREATE;

    sendPacket(&header, &docDesc);

}

void LogAppServer::removeDocument(DocumentDescriptor docDesc)
{
    qDebug() << "not implemented";
}

void LogAppServer::saveDocument(DocumentDescriptor docDesc)
{
    NetworkHeader header;

    header.dataSize = sizeof(DocumentDescriptor);
    header.type = PT_DOC_SAVE;

    sendPacket(&header, &docDesc);
}

void LogAppServer::onConnectionEstablished()
{
    qDebug() << "connection established";
    status = SS_CONNECTED;
    emit connected();
}

void LogAppServer::onConnectionLost()
{
    qDebug() << "disconnected" << socket.errorString();
    status = SS_DISCONNECTED;
    socket.disconnectFromHost();
    socket.abort();
    emit disconnected("Connection lost");
}

void LogAppServer::onConnectionError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "connection error: " << socketError;
    status = SS_DISCONNECTED;
    socket.disconnectFromHost();
    socket.abort();
    emit disconnected("Error: " + socket.errorString());
}

void LogAppServer::onConnectionSslError(QList<QSslError> errors)
{
    qDebug() << "connection ssl error: " << errors;
    socket.disconnectFromHost();
    socket.abort();
}

void LogAppServer::onSocketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "state changed: " << state;
    switch(state) {
    case QAbstractSocket::ConnectedState:
//        socket.startClientEncryption();
        break;
    default:
        break;
    }
}

void RemoteDB::fillItemDescriptor(ItemDescriptor &id, const LogItem *item)
{
    id.id = item->getId();
    id.docId = doc->getId();
    id.prevId = item->getPrev()?item->getPrev()->getId():0;
    id.parentId = item->getParent()->getId();
    id.done = item->isDone()?1:0;
    id.folded = item->isFolded()?1:0;
}

RemoteDB::RemoteDB(LogAppServer *server, ClientDocument *doc)
    : server(server)
    , doc(doc)
    , pendingRequests(0)
{
    connect(doc, SIGNAL(itemCreated(LogItem*)), this, SLOT(onItemAdded(LogItem*)));
    connect(doc, SIGNAL(itemDeleted(LogItem*)), this, SLOT(onItemDeleted(LogItem*)));
    connect(doc, SIGNAL(itemTextChanged(LogItem*)), this, SLOT(onItemTextChanged(LogItem*)));
    connect(doc, SIGNAL(itemModified(LogItem*)), this, SLOT(onItemModified(LogItem*)));
    connect(doc, SIGNAL(itemDoneChanged(LogItem*)), this, SLOT(onItemDoneChanged(LogItem*)));
    connect(doc, SIGNAL(docSaved()), this, SLOT(onDocumentSaved()));
}

void RemoteDB::onItemListReceived(uint64_t parentId, ItemDescriptor *ids, uint count)
{
    pendingRequests--;
    for (size_t i = 0; i < count; i++) {
        ItemDescriptor &itemd = ids[i];
        LogItem *item = doc->findItemById(itemd.id);
        if (item) {
            qDebug() << "ERROR: item already exists";
//            return;
        }
        LogItem *parent = doc->findItemById(parentId);
        if (!parent) {
            qDebug() << "ERROR: parent not exists";
            return;
        }
        item = new LogItem(doc, parent, itemd.id);
        item->setState(ItemState::NOT_PRESENT);
        item->setDone(itemd.done);
        item->setFolded(itemd.folded);

        LogItem *prev = (itemd.prevId != 0)?doc->findItemById(itemd.prevId):nullptr;
        doc->addItem(item, parent, prev);

        server->getItemChildren(doc->getId(), itemd.id, this);
        pendingRequests++;

        qDebug() << "send request for item data: " << itemd.id;
        server->getItemData(doc->getId(), itemd.id, this);
        pendingRequests++;
        item->setState(ItemState::DOWNLOADING);
    }
    if (!pendingRequests) {
        qDebug() << "LOADING DONE";
//        doc->setStatus(DS_);
    }
}

void RemoteDB::onItemReceived(ServerItemData data)
{
    pendingRequests--;
    LogItem *item = doc->findItemById(data.itemId);
    if (!item) {
        qDebug() << "ERROR: item not exists";
        return;
    }
    item->setText(data.text);
    item->setState(ItemState::PRESENT);
    emit doc->itemTextChanged(item);
    if (!pendingRequests) {
        qDebug() << "LOADING DONE";
//        doc->setStatus(DS_);
    }
}

void RemoteDB::onRequestAnsverReceived(uint64_t requestId, void *data)
{

}

void RemoteDB::start()
{
    pendingRequests = 1;
    server->getItemChildren(doc->getId(), ClientDocument::ROOT_ITEM_ID, this);
}

void RemoteDB::onItemAdded(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->addItem(id);
}

void RemoteDB::onItemTextChanged(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->changeItem(id, item->getText());
}

void RemoteDB::onItemModified(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->moveItem(id);
}

void RemoteDB::onItemDeleted(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->removeItem(id);
}

void RemoteDB::onItemDoneChanged(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->setItemDone(id);
}

void RemoteDB::onItemFoldChanged(LogItem *item)
{
    ItemDescriptor id;
    fillItemDescriptor(id, item);
    server->setItemFolded(id);
}

void RemoteDB::onDocumentSaved()
{
    DocumentDescriptor dd;
    dd.id = doc->getId();
    server->saveDocument(dd);
}

//void RemoteDB::saveTree(LogItem *rootItem)
//{

//}

//void RemoteDB::loadTree(LogControl *control, LogItem *rootItem)
//{

//}
