#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "../network_def.h"
#include "core.h"
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
    std::vector<LogControl *>docs;

protected:
    void sendDocumentList(NetworkHeader *header, QTcpSocket *connection);

public:
    TodoServer(const std::vector<LogControl*> docs);

    void start();
    void sendPacket(NetworkHeader *header, const void *data);
    void readPacket();
    void sendItem(LogItem *item);
    void sendChildrenIds(uint64_t itemId, std::vector<uint64_t> &ids);
    void sendAction();

signals:
    void createItem(CreateItemData data);
    void moveItem();
    void changeItemText(ChangeItemData data);
    void removeItem(RemoveItemData data);
    void itemRequested(uint64_t id);
    void childrenIdsRequested(uint64_t parentId);

protected slots:
    void incomingMessage();
    void onNewConnection();
};

#endif // SERVER_H
