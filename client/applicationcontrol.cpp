#include "applicationcontrol.h"

#include "logappserver.h"

#define DEFAULT_APP_FOLDER_NAME ".todo/test"

ApplicationControl::ApplicationControl()
    : server(new LogAppServer())
    , state(ApplicationState::START)
    , connectionState(ApplicationConnectionState::JUST_STARTED)
{
    connect(server, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(server, SIGNAL(disconnected(QString)), this, SLOT(onServerDisconnected(QString)));
    connect(server, SIGNAL(docListReceived(std::vector<std::pair<uint64_t,QString> >)), this, SLOT(onDocListReceived(std::vector<std::pair<uint64_t,QString> >)));

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

QString getRandomFileName(size_t length)
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
    QString fullFileName = appDir.path() + QDir::separator() + getRandomFileName(12) + ".xml";
    LogControl *newDoc = nullptr;
    if (type == DT_LOCAL || type == DT_CACHED) {
        QFile file(fullFileName);
        if (file.exists()) {
            qDebug() << "error: file exists";
            return false;
        }

        file.open(QIODevice::ReadWrite);
        if (!file.isOpen())
            return false;
        file.close();

        DB *db = new XmlDB(fullFileName);
        newDoc = new LogControl(db, name, LogControl::getNextDocId());
    }
    if (type == DT_REMOTE || type == DT_CACHED) {
        if (!newDoc)
            newDoc = new LogControl(nullptr, name, LogControl::getNextDocId());
//        server->createDocument();
        RemoteDB *rdb = new RemoteDB(server, newDoc);
        newDoc->setServerDB(rdb, type);
    }
    if (newDoc) {
        newDoc->loadData();
        newDoc->save();
        return true;
    }
    return false;

}

void ApplicationControl::start()
{
    QString appDirectoryName = QDir::homePath() + QDir::separator() + DEFAULT_APP_FOLDER_NAME;
    qDebug() << appDirectoryName;
    appDir = QDir(appDirectoryName);
    if (!appDir.exists()) {
        appDir.mkpath(appDirectoryName);
    }

    QStringList filters;
    filters.append("*.xml");
    QStringList fileList = appDir.entryList(filters);
    qDebug() << fileList;

    for (int i = 0; i < fileList.size(); i++) {
        auto s = fileList[i];
        QString fileName = appDir.path() + QDir::separator() + s;

        DB *db = new XmlDB(fileName);
        LogControl *doc = new LogControl(db, s, i);
        doc->loadData();

        emit documentAdded(doc);
    }
}

void ApplicationControl::onServerConnected()
{
    switch (connectionState)
    {
    case ApplicationConnectionState::JUST_STARTED:
    case ApplicationConnectionState::CONNECTION_ERROR:
        server->getDocList();
        break;
    case ApplicationConnectionState::CONNECTION_LOST:

        break;
    default:
        break;
    }
}

void ApplicationControl::onServerDisconnected(QString reason)
{
    emit setConnectionStatus(reason);
}

void ApplicationControl::onDocListReceived(std::vector<std::pair<uint64_t, QString> > docs)
{
    for (auto s: docs) {
        LogControl *doc = new LogControl(nullptr, s.second, s.first);
        qDebug() << "created remote doc with id: " << s.first;
        RemoteDB *rdb = new RemoteDB(server, doc);
        doc->setServerDB(rdb, DT_REMOTE);
        doc->loadData();

        emit documentAdded(doc);
    }
}
