#ifndef APPLICATIONCONTROL_H
#define APPLICATIONCONTROL_H

#include <vector>

#include <QObject>
#include <QDir>
#include <QTimer>
#include <QThread>

#include "core.h"

class ClientDocument;
class GuiControl;
class LogAppServer;

enum class ApplicationState
{
    START = 0,
    RECEIVING_ITEMS,
    NORMAL,
    SEARCH
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
    ClientDocument *currentDocument;
    GuiControl *guiControl;

    std::vector<ClientDocument *> localDocs;
    std::vector<ClientDocument *> remoteDocs;

    QDir appDir;
    QTimer connectionTimer;
    QThread serverThread;

    void leaveSearch();
    void doSearch(QString request);

protected slots:
    void serverPooling();

public:
    ApplicationControl(GuiControl *guiControl);
    ~ApplicationControl();

    bool createNewDocument(QString name, DocumentType type);
    void start();
    bool canExit();

public slots:
    void onServerConnected();
    void onServerDisconnected(QString reason);
    void onDocListReceived(std::vector<std::pair<uint64_t, QString>> docs);
    void onNewDocumentOnServer(uint64_t id, QString name);
    void search(QString request);
    void setCurrentDocument(ClientDocument *doc);

signals:
    void createdNewDocument(ClientDocument *doc);
    void documentAdded(ClientDocument *doc);
    void setConnectionStatus(QString);
    //void documentDeleted(LogCon)

};

#endif // APPLICATIONCONTROL_H
