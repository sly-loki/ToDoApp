#include "applicationcontrol.h"

#include <QRegularExpression>

#include "logappserver.h"
#include "guicontrol.h"
#include "../server/server.h"

#define DEFAULT_APP_FOLDER_NAME ".todo/test"

ApplicationControl::ApplicationControl(GuiControl *guiControl)
    : server(new LogAppServer("local", "localhost", DEBUG_PORT))
    , state(ApplicationState::NORMAL)
    , connectionState(ApplicationConnectionState::JUST_STARTED)
    , currentDocument(nullptr)
    , guiControl(guiControl)
{
    connect(server, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(server, SIGNAL(disconnected(QString)), this, SLOT(onServerDisconnected(QString)));
    connect(server, SIGNAL(docListReceived(std::vector<std::pair<uint64_t,QString> >)), this, SLOT(onDocListReceived(std::vector<std::pair<uint64_t,QString> >)));
    connect(server, SIGNAL(docCreatedOnServer(uint64_t,QString)), this, SLOT(onNewDocumentOnServer(uint64_t,QString)));

    connectionTimer.setInterval(5000);
    connectionTimer.setSingleShot(true);
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(serverPooling()));

    server->connectToServer();
    serverPooling();
}

ApplicationControl::~ApplicationControl()
{
    delete server;
}

void ApplicationControl::leaveSearch()
{
    ClientDocument *doc = currentDocument;
    ClientItem *item = doc->getNextItemInTree(doc->getRootItem());

    while(item) {
        guiControl->showItem(item);
        item = doc->getNextItemInTree(item);
    }
}

void ApplicationControl::doSearch(QString request)
{
    qDebug() << "search: " << request;
    ClientDocument *doc = currentDocument;
    ClientItem *item = doc->getNextItemInTree(doc->getRootItem());

    QRegularExpression re(request);

    while(item) {

        QRegularExpressionMatch match = re.match(item->getText());
        if (match.hasMatch()) {
            qDebug() << "found: " << item->getText();
            guiControl->showItem(item);
        } else {
            guiControl->hideItem(item);
        }
        item = doc->getNextItemInTree(item);
    }
}

void ApplicationControl::serverPooling()
{
    if (server->getStatus() == SS_CONNECTED) {
        emit setConnectionStatus("connected");
    } else {
//        if (server->getStatus() == SS_DISCONNECTED)
        server->connectToServer();
        connectionTimer.start();
    }
}

static QString getRandomFileName(size_t length)
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    QTime currentTime = QTime::currentTime();
    qsrand(currentTime.hour() + currentTime.second() + currentTime.minute() + currentTime.msec());
    QString randomString;
    for(size_t i=0; i<length; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

bool ApplicationControl::createNewDocument(QString name, DocumentType type)
{
    //TODO: creation should accept list of servers on which document will exists or smth like this

    DocumentDescriptor desc;
    memcpy(desc.name, name.toStdString().c_str(), name.toStdString().size());
    desc.name[name.toStdString().size()] = 0;
    server->createDocument(desc);
    return true;
}

#define DEFAULT_LOCAL_SERVER_FOLDER_NAME ".todo/server_test"

void ApplicationControl::start()
{
    //running local server
    TodoServer *server;
    QString appDirectoryName = QDir::homePath() + QDir::separator() + DEFAULT_LOCAL_SERVER_FOLDER_NAME;
    server = new TodoServer(appDirectoryName);
    server->moveToThread(&serverThread);
    connect(&serverThread, SIGNAL(started()), server, SLOT(start()));
    serverThread.start();
}

bool ApplicationControl::canExit()
{
    for(ClientDocument *doc: localDocs) {
        if (doc->getModified()) {
            return false;
        }
    }
    return true;
}

void ApplicationControl::onServerConnected()
{
    switch (connectionState)
    {
    case ApplicationConnectionState::JUST_STARTED:
    case ApplicationConnectionState::CONNECTION_ERROR:
        server->getDocList();
        connectionState = ApplicationConnectionState::CONNECTED;
        break;
    case ApplicationConnectionState::CONNECTION_LOST:
        connectionState = ApplicationConnectionState::CONNECTED;
        break;
    default:
        break;
    }
}

void ApplicationControl::onServerDisconnected(QString reason)
{
    if (connectionState == ApplicationConnectionState::CONNECTED)
        connectionState = ApplicationConnectionState::CONNECTION_LOST;
    emit setConnectionStatus(reason);
}

void ApplicationControl::onDocListReceived(std::vector<std::pair<uint64_t, QString> > docs)
{
    for (auto s: docs) {
        ClientDocument *doc = new ClientDocument(s.second, s.first);
        remoteDocs.push_back(doc);
        qDebug() << "created remote doc with id: " << s.first;
        RemoteDB *rdb = new RemoteDB(server, doc);
        doc->setServerDB(rdb, DT_REMOTE);
        doc->loadData();
        emit documentAdded(doc);
    }
}

void ApplicationControl::onNewDocumentOnServer(uint64_t id, QString name)
{
    ClientDocument *doc = new ClientDocument(name, id);
    remoteDocs.push_back(doc);
    qDebug() << "created remote doc with id: " << id;
    RemoteDB *rdb = new RemoteDB(server, doc);
    doc->setServerDB(rdb, DT_REMOTE);
    doc->loadData();
    emit documentAdded(doc);
}

void ApplicationControl::search(QString request)
{
    switch (state)
    {
    case ApplicationState::NORMAL:
        if (request != "") {
            state = ApplicationState::SEARCH;
            doSearch(request);
        }
        break;
    case ApplicationState::SEARCH:
        if (request == "") {
            state = ApplicationState::NORMAL;
            leaveSearch();
        } else {
            doSearch(request);
        }
        break;
    }
}

void ApplicationControl::setCurrentDocument(ClientDocument *doc)
{
    currentDocument = doc;
}
