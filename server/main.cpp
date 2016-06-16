#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <memory>
#include "../network_def.h"

class TodoServer: public QObject
{
    Q_OBJECT
    QTcpServer server;
    QTcpSocket *clientConnection;

public:
    TodoServer()
        : clientConnection(nullptr)
    {
        connect(&server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }

    void start()
    {

    }

public slots:
    void incomingMessage()
    {

    }

    void onNewConnection()
    {
        if (clientConnection)
            delete clientConnection;
        clientConnection = server.nextPendingConnection();
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingMessage()));

    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TodoServer todoServer;

    todoServer.start();

    return a.exec();
}

#include "main.moc"
