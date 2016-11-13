#ifndef LOGAPPSERVER_H
#define LOGAPPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QThread>

#include <map>

#include "network_def.h"
#include "core.h"

class ClientItem;
struct NetworkHeader;

enum ServerStatus
{
    SS_CONNECTING,
    SS_CONNECTED,
    SS_DISCONNECTED,
    SS_ERROR
};

class ServerThread: public QThread
{
public:
    ServerThread();

protected:
    void run();
};

struct ServerRequest
{

};

struct ServerItemData
{
    uint64_t itemId;
    uint64_t parentId;
    QString text;
    bool done;
    bool folded;
};

struct ServerAction
{
    uint64_t docId;
    uint32_t type;
    uint32_t dataSize;
    uint8_t *data;
};

class NetworkClient
{
public:
    virtual void requestAnsvered(ServerAction action, char *data) = 0;
    virtual ~NetworkClient() = default;
};

class RemoteDB;

class LogAppServer : public QObject
{
    Q_OBJECT
    QSslSocket socket;
    uint64_t request_id;
    ServerStatus status;
    std::map<uint64_t, RemoteDB *> requests;

    void sendPacket(NetworkHeader *header, const void *data);
    void sendPacketSync(NetworkHeader *header, const void *data);

protected slots:
    void readData();
    void onConnectionEstablished();
    void onConnectionLost();
    void onConnectionError(QAbstractSocket::SocketError socketError);
    void onConnectionSslError(QList<QSslError> errors);
    void onSocketStateChanged(QAbstractSocket::SocketState state);

public:
    explicit LogAppServer(QObject *parent = 0);
    void connectToServer();

    bool ping();
    bool saveItem(ClientItem *item);
    bool saveTree(ClientItem *root);

    void getItemList();
    void getItemData(uint64_t docId, uint64_t id, RemoteDB *db);
    void getItemChildren(uint64_t docId, uint64_t id, RemoteDB *db);

    void getDocList();
    ServerStatus getStatus() const {return status;}

public slots:
    void addItem(ItemDescriptor item);
    void changeItem(ItemDescriptor item, QString text);
    void moveItem(ItemDescriptor item);
    void removeItem(ItemDescriptor item);
    void sendAction(ServerAction action);
    void setItemDone(ItemDescriptor id);
    void setItemFolded(ItemDescriptor id);

    void createDocument(DocumentDescriptor docDesc);
    void removeDocument(DocumentDescriptor docDesc);
    void saveDocument(DocumentDescriptor docDesc);

signals:

    void connected();
    void disconnected(QString);

    void receivedRequest();
    void docListReceived(std::vector<std::pair< uint64_t, QString>>);
    void docCreatedOnServer(uint64_t, QString);
//    void itemListReceived(uint64_t *ids, uint count);
//    void itemReceived(ServerItemData data);
//    void itemChildrenReceived(uint64_t parentId, uint64_t *ids, uint count);

public slots:
};

class RemoteDB : public QObject
{
    Q_OBJECT

    enum RemoteDbState
    {
        RDB_CREATED = 0,
        RDB_SYNCHING,
        RDB_SYNCHED
    };

    LogAppServer *server;
    ClientDocument *doc;
    RemoteDbState state;
    size_t pendingRequests;

    void fillItemDescriptor(ItemDescriptor &id, const ClientItem *item);

public:
    RemoteDB(LogAppServer *server, ClientDocument *doc);

    void onItemListReceived(uint64_t parentId, ItemDescriptor *ids, uint count);
    void onItemReceived(ServerItemData data);
    void onRequestAnsverReceived(uint64_t requestId, void *data);
    void start();

    //    virtual void loadTree(LogControl *control, LogItem *rootItem) override;

public slots:
    void onItemAdded(ClientItem *item);
//    void onItemStateChanged(LogItem *item);
    void onItemTextChanged(ClientItem *item);
    void onItemModified(ClientItem *item);
    void onItemDeleted(ClientItem *item);
//    void onItemFocused(LogItem *item);
    void onItemDoneChanged(ClientItem *item);
    void onItemFoldChanged(ClientItem *item);

    void onDocumentSaved();

signals:
//    void

};

#endif // LOGAPPSERVER_H
