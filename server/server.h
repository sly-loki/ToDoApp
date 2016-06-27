#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "../network_def.h"
//#include "core.h"

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

public:
    TodoServer();

    void start();
    void sendPacket(NetworkHeader *header, const void *data);
    void readPacket();

signals:
    void createItem(CreateItemData data);
    void moveItem();
    void changeItemText(ChangeItemData data);
    void removeItem(RemoveItemData data);

protected slots:
    void incomingMessage();
    void onNewConnection();
};

#endif // SERVER_H
