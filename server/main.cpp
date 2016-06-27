#include <QCoreApplication>
#include <QDebug>
#include <QLockFile>
#include <QDir>

#include <iostream>

#include "server.h"

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
    LogControl control(&db);

    todoServer.setControl(&control);
    control.loadData();
    todoServer.start();

    return a.exec();
}
