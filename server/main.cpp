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
        if (!server.listen(QHostAddress::Any, 12054)) {
            qDebug() << "server listen error";
            return;
        }
    }

public slots:
    void incomingMessage()
    {
        NetworkHeader packet;
        if (clientConnection->read(&packet, sizeof(packet)) != sizeof(packet)) {
            qDebug() << "read error";
            return;
        }
        switch (packet.type) {
        case PT_ITEM_CREATED:
            qDebug() << "item created message";
            break;
        case PT_GET_ALL_ITEMS:
            qDebug() << "request for all item";
            break;
        }
    }

    void onNewConnection()
    {
        if (clientConnection)
            delete clientConnection;
        clientConnection = server.nextPendingConnection();
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingMessage()));
        qDebug() << "new connection established";
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
