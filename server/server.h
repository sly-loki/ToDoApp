#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QTcpSocket>
#include <QDir>

#include "network_def.h"
#include "../server/servercore.h"
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

class TodoServer: public QTcpServer
{
    Q_OBJECT
    QSslSocket *clientConnection;
    std::map<uint64_t, ServerDocument *>docs;

    QDir storageDir;

protected:

    virtual void incomingConnection(qintptr handle);

    void sendDocumentList(NetworkHeader *header, QTcpSocket *connection);
    void sendItem(NetworkHeader *header);
    void sendChildrenIds(NetworkHeader *header);

    void sendPacket(NetworkHeader *header, const void *data);
    void readPacket();

    void sendResponse(uint64_t requestID, uint16_t *response);

    uint16_t createItem(ItemDescriptor item);
    uint16_t removeItem(ItemDescriptor item);
    uint16_t setItemText(NetworkHeader *header, QString text);
    uint16_t moveItem(ItemDescriptor id);
    uint16_t setItemDone(ItemDescriptor id);
    uint16_t setItemFold(ItemDescriptor id);

    uint16_t saveDocument(DocumentDescriptor desc);
    uint16_t createDocument(DocumentDescriptor &desc);
    uint16_t removeDocument(DocumentDescriptor desc);

protected slots:
    void incomingMessage();
    void onNewConnection();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketSslError();
    void onSocketStateChanged(QAbstractSocket::SocketState state);

public:
    TodoServer(QString storageFolderName);
public slots:
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
