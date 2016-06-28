#include <QCoreApplication>
#include <QDebug>
#include <QLockFile>
#include <QDir>

#include <iostream>

#include "server.h"
#include "../storage.h"
#include "core.h"

int main(int argc, char *argv[])
{
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/todo_server.lock");

    if(!lockFile.tryLock(100)){
        std::cout << "Error: another instance of server already lanched" << std::endl;
        return 1;
    }

    QCoreApplication a(argc, argv);
    TodoServer todoServer;
    XmlDB db("/tmp/test.xml");
    LogControl control(&db, &todoServer);

    QObject::connect(&todoServer, SIGNAL(createItem(CreateItemData)), &control, SLOT(createItem(CreateItemData)));
    QObject::connect(&todoServer, SIGNAL(changeItemText(ChangeItemData)), &control, SLOT(changeItem(ChangeItemData)));
    QObject::connect(&todoServer, SIGNAL(removeItem(RemoveItemData)), &control, SLOT(removeItem(RemoveItemData)));
    QObject::connect(&todoServer, SIGNAL(childrenIdsRequested(uint64_t)), &control, SLOT(sendChildrenIds(uint64_t)));
    QObject::connect(&todoServer, SIGNAL(itemRequested(uint64_t)), &control, SLOT(sendItem(uint64_t)));

    control.loadData();
    todoServer.start();

    return a.exec();
}
