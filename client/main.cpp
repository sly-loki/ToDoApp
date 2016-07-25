#include "mainwindow.h"
#include <QApplication>
#include <QLockFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/todo_client_2.lock");

    if(!lockFile.tryLock(100)){
        qDebug() << "Error: another instance of client already lanched";
        return 1;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
