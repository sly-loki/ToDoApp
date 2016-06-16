#ifndef LOGAPPSERVER_H
#define LOGAPPSERVER_H

#include <QObject>
#include <QTcpServer>

class LogItem;

enum ServerStatus
{
    SS_OK,
    SS_DISCONNECTED,
    SS_ERROR
};

class LogAppServer : public QObject
{
    Q_OBJECT
    QTcpServer server;
public:
    explicit LogAppServer(QObject *parent = 0);
    bool connect();
    ServerStatus getStatus();

    bool SaveItem(LogItem *item);
    bool SaveTree(LogItem *root);

signals:

public slots:
};

#endif // LOGAPPSERVER_H
