#ifndef APPLICATIONCONTROL_H
#define APPLICATIONCONTROL_H

#include <vector>

#include <QObject>
#include <QDir>
#include <QTimer>

#include "core.h"

class ClientDocument;
class GuiControl;
class LogAppServer;

enum class ApplicationState
{
    START = 0,
    RECEIVING_ITEMS,
    NORMAL
};

enum class ApplicationConnectionState
{
    CONNECTED = 0,
    CONNECTION_LOST,
    CONNECTION_ERROR,
    JUST_STARTED
};

class ApplicationControl : public QObject
{
    Q_OBJECT
    LogAppServer *server;
    ApplicationState state;
    ApplicationConnectionState connectionState;

    std::vector<ClientDocument *> localDocs;
    std::vector<ClientDocument *> remoteDocs;

    QDir appDir;
    QTimer connectionTimer;

protected slots:
    void serverPooling();

public:
    ApplicationControl();
    ~ApplicationControl();

    bool createNewDocument(QString name, DocumentType type);
    void start();
    bool canExit();

public slots:
    void onServerConnected();
    void onServerDisconnected(QString reason);
    void onDocListReceived(std::vector<std::pair<uint64_t, QString>> docs);

signals:
    void createdNewDocument(ClientDocument *doc);
    void documentAdded(ClientDocument *doc);
    void setConnectionStatus(QString);
    //void documentDeleted(LogCon)

};

#endif // APPLICATIONCONTROL_H
