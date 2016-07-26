#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "network_def.h"
#include "core.h"
#include <map>
#include <vector>

class LogItem;

struct CreateItemData {
    uint64_t parentId;
    uint64_t prevItemId;
    QString text;
};

struct ChangeItemData {
    uint64_t itemId;
    QString text;
};

struct RemoveItemData {
    uint64_t itemId;
};

class TodoServer: public QObject
{
    Q_OBJECT
    QTcpServer server;
    QTcpSocket *clientConnection;
    std::map<uint64_t, LogControl *>docs;

protected:
    void sendDocumentList(NetworkHeader *header, QTcpSocket *connection);
    void sendItem(NetworkHeader *header);
    void sendChildrenIds(NetworkHeader *header);
    void sendPacket(NetworkHeader *header, const void *data);
    void readPacket();

    void createItem(NetworkHeader *header, const char *data);

protected slots:
    void incomingMessage();
    void onNewConnection();

public:
    TodoServer(const std::vector<LogControl*> docs);
    void start();

signals:
//    void createItem(CreateItemData data);
//    void moveItem();
//    void changeItemText(ChangeItemData data);
//    void removeItem(RemoveItemData data);
//    void itemRequested(uint64_t id);
//    void childrenIdsRequested(uint64_t parentId);
};

#endif // SERVER_H
