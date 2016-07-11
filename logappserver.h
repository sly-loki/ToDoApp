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
    SS_OK,
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

class LogAppServer : public QObject
{
    Q_OBJECT
    QTcpSocket socket;
    uint64_t request_id;

    void sendPacket(NetworkHeader *header, const void *data);

protected slots:
    void readData();

public:
    explicit LogAppServer(QObject *parent = 0);
    bool connectToServer();
    ServerStatus getStatus();

    bool ping();
    bool saveItem(LogItem *item);
    bool saveTree(LogItem *root);

    void getItemList();
    void getItemData(uint64_t id);
    void getItemChildern(uint64_t docId, uint64_t id);

    void getDocList();

public slots:
    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void sendAction(ServerAction action);

signals:

    void connected();

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
