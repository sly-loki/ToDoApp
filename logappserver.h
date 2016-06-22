#ifndef LOGAPPSERVER_H
#define LOGAPPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>

#include <map>

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

public slots:
    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void sendAction(ServerAction action);

signals:
    void receivedRequest();
    void itemListReceived(uint64_t *ids, uint count);
    void itemReceived(ServerItemData data);

public slots:
};

#endif // LOGAPPSERVER_H
