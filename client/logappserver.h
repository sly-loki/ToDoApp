#ifndef LOGAPPSERVER_H
#define LOGAPPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>

#include <map>

#include "storage.h"

class LogItem;
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
};

struct ServerAction
{
    uint32_t type;
};

class NetworkClient
{
public:
    virtual void requestAnsvered(ServerAction action, char *data) = 0;
    virtual ~NetworkClient() = default;
};

class LogAppServer : public QObject
{
    Q_OBJECT
    QTcpSocket socket;
    uint64_t request_id;
    ServerStatus status;

    void sendPacket(NetworkHeader *header, const void *data);
    void sendPacketSync(NetworkHeader *header, const void *data);

protected slots:
    void readData();

public:
    explicit LogAppServer(QObject *parent = 0);
    void connectToServer();

    bool ping();
    bool saveItem(LogItem *item);
    bool saveTree(LogItem *root);

    void getItemList();
    void getItemData(uint64_t id);
    void getItemChildern(uint64_t docId, uint64_t id);

    void getDocList();
    ServerStatus getStatus() const {return status;}

public slots:
    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void sendAction(ServerAction action);
    void onConnectionEstablished();
    void onConnectionLost();
    void onConnectionError(QAbstractSocket::SocketError socketError);

signals:

    void connected();
    void disconnected(QString);

    void receivedRequest();
    void docListReceived(std::vector<QString>);
    void itemListReceived(uint64_t *ids, uint count);
    void itemReceived(ServerItemData data);
    void itemChildrenReceived(uint64_t parentId, uint64_t *ids, uint count);

public slots:
};

class RemoteDB: public DB
{

    LogAppServer *server;
    QString docName;

public:
    RemoteDB(LogAppServer *server, QString docName);
    virtual void saveTree(LogItem *rootItem) override;
    virtual void loadTree(LogControl *control, LogItem *rootItem) override;
};

#endif // LOGAPPSERVER_H
