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
            }
        }
            break;
        case PT_GET_CHILDREN: {
            qDebug() << "get children items response";
            size_t itemCount = header.dataSize/sizeof(uint64_t);
            uint64_t *ids = nullptr;

            if (itemCount != 0) {
                ids = new uint64_t[itemCount];
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
            } else {
                qDebug() << "no children items";
            }

            auto it = requests.find(header.requestID);
            if (it != requests.end()) {
                RemoteDB *db = (*it).second;
                db->onItemListReceived(header.itemId, ids, itemCount);
                requests.erase(it);
            }

            delete[] ids;
        }
            break;
        case PT_GET_DOC_LIST: {
            qDebug() << "get doc list";
            std::vector<QString> docs;
            int count = header.dataSize / sizeof(DocumentDescriptor);
            DocumentDescriptor *descs = new DocumentDescriptor[count];
            size_t readed = socket.read((char *)descs, header.dataSize);
            if (readed != header.dataSize) {
                qDebug() << "ERROR: ";
                delete[] descs;  //fix
                return;
            }
            for (int i = 0; i < count; i++) {
                docs.push_back(QString((char *)(descs[i].name)));
            }
            emit docListReceived(docs);
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
    connect(&socket, SIGNAL(connected()), this, SLOT(onConnectionEstablished()));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(onConnectionLost()));
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onConnectionError(QAbstractSocket::SocketError)));
}

void LogAppServer::connectToServer()
{
//    socket.abort();
    QAbstractSocket::SocketState state = socket.state();
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        socket.connectToHost("localhost", DEBUG_PORT);
        status = SS_CONNECTING;
        break;
    case QAbstractSocket::ConnectedState:
        status = SS_CONNECTED;
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
    requests[request_id] = db;
    request_id++;

    header.docId = docId;
    header.itemId = id;
    header.dataSize = 0;
    header.type = PT_GET_ITEM;

    sendPacket(&header, nullptr);
}

void LogAppServer::getItemChildern(uint64_t docId, uint64_t id, RemoteDB *db)
{
    NetworkHeader header;
    header.requestID = request_id;
    requests[request_id] = db;
    request_id++;

    header.docId = docId;
    header.itemId = id;
    header.dataSize = 0;
    header.type = PT_GET_CHILDREN;

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

void LogAppServer::onConnectionEstablished()
{
    status = SS_CONNECTED;
    emit connected();
}

void LogAppServer::onConnectionLost()
{
    status = SS_DISCONNECTED;
    socket.disconnectFromHost();
    socket.abort();
    emit disconnected("Connection lost");
}

void LogAppServer::onConnectionError(QAbstractSocket::SocketError socketError)
{
    status = SS_DISCONNECTED;
    socket.disconnectFromHost();
    socket.abort();
    emit disconnected("Error: " + socket.errorString());
}

RemoteDB::RemoteDB(LogAppServer *server, LogControl *doc)
    : server(server)
    , doc(doc)
{

}

void RemoteDB::onItemListReceived(uint64_t parentId, uint64_t *ids, uint count)
{
    for (int i = 0; i < count; i++) {
        uint64_t id = ids[i];
        LogItem *item = doc->findItemById(id);
        if (item) {
            qDebug() << "ERROR: item already exists";
            return;
        }
        LogItem *parent = doc->findItemById(parentId);
        if (!parent) {
            qDebug() << "ERROR: parent not exists";
            return;
        }
        item = new LogItem(doc, parent, id);
        item->setState(IS_NOT_PRESENT);
        doc->addItem(item, parent);

        server->getItemData(doc->getId(), id, this);
        item->setState(IS_DOWNLOADING);
    }
}

void RemoteDB::onItemReceived(ServerItemData data)
{
    LogItem *item = doc->findItemById(data.itemId);
    if (!item) {
        qDebug() << "ERROR: item not exists";
        return;
    }
    item->setText(data.text);
    item->setState(IS_PRESENT);
    emit doc->itemTextChanged(item);
}

void RemoteDB::start()
{
    server->getItemChildern(doc->getId(), LogControl::ROOT_ITEM_ID, this);
}

//void RemoteDB::saveTree(LogItem *rootItem)
//{

//}

//void RemoteDB::loadTree(LogControl *control, LogItem *rootItem)
//{

//}
