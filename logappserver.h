#ifndef LOGAPPSERVER_H
#define LOGAPPSERVER_H

#include <QObject>
#include <QTcpSocket>

class LogItem;
struct NetworkHeader;

enum ServerStatus
{
    SS_OK,
    SS_DISCONNECTED,
    SS_ERROR
};

class LogAppServer : public QObject
{
    Q_OBJECT
    QTcpSocket socket;

    void sendPacket(NetworkHeader *header, const void *data);

protected slots:
    void readData();

public:
    explicit LogAppServer(QObject *parent = 0);
    bool connectToServer();
    ServerStatus getStatus();

    bool ping();
    bool SaveItem(LogItem *item);
    bool SaveTree(LogItem *root);

public slots:
    void addItem(LogItem *item);
    void removeItem(LogItem *item);

signals:

public slots:
};

#endif // LOGAPPSERVER_H
