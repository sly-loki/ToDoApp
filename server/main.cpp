#include <QCoreApplication>
#include <QDebug>
#include <QLockFile>
#include <QDir>
#include <QStringList>

#include <iostream>

#include "server.h"
#include "storage.h"
#include "core.h"

#define DEFAULT_APP_FOLDER_NAME ".todo/server_test"

int main(int argc, char *argv[])
{
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/todo_server.lock");

    if(!lockFile.tryLock(100)){
        std::cout << "Error: another instance of server already lanched" << std::endl;
        return 1;
    }

    QString appDirectoryName = QDir::homePath() + QDir::separator() + DEFAULT_APP_FOLDER_NAME;
    QDir appDir = QDir(appDirectoryName);
    if (!appDir.exists()) {
        appDir.mkpath(appDirectoryName);
    }

    QStringList filters;
    filters.append("*.xml");
    QStringList fileList = appDir.entryList(filters);
    qDebug() << fileList;

    std::vector<LogControl *> docs;
    for (auto s: fileList) {
        QString fileName = appDir.path() + QDir::separator() + s;

        DB *db = new XmlDB(fileName);
        LogControl *control = new LogControl(db, s);
        control->loadData();
        docs.push_back(control);
    }

    QCoreApplication a(argc, argv);
    TodoServer todoServer(docs);
//    XmlDB db("/tmp/test.xml");
//    LogControl control(&db, &todoServer);

//    QObject::connect(&todoServer, SIGNAL(createItem(CreateItemData)), &control, SLOT(createItem(CreateItemData)));
//    QObject::connect(&todoServer, SIGNAL(changeItemText(ChangeItemData)), &control, SLOT(changeItem(ChangeItemData)));
//    QObject::connect(&todoServer, SIGNAL(removeItem(RemoveItemData)), &control, SLOT(removeItem(RemoveItemData)));
//    QObject::connect(&todoServer, SIGNAL(childrenIdsRequested(uint64_t)), &control, SLOT(sendChildrenIds(uint64_t)));
//    QObject::connect(&todoServer, SIGNAL(itemRequested(uint64_t)), &control, SLOT(sendItem(uint64_t)));

//    control.loadData();
    todoServer.start();

    return a.exec();
}
