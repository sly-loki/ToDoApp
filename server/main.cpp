#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <memory>
#include "../network_def.h"
#include "../core.h"

class TodoServer: public QObject
{
    Q_OBJECT
    QTcpServer server;
    QTcpSocket *clientConnection;
    LogControl *control;

public:
    TodoServer()
        : clientConnection(nullptr)
        , control(nullptr)
    {
        connect(&server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }

    void start()
    {
        if (!server.listen(QHostAddress::Any, DEBUG_PORT)) {
            qDebug() << "server listen error";
            return;
        }
    }

    void setControl(LogControl *control)
    {
        this->control = control;
    }

    void readPacket()
    {

    }

public slots:
    void incomingMessage()
    {
        if (!control) {
            qDebug() << "LogControl not set";
            return;
        }
        NetworkHeader packet;
        if (clientConnection->read((char *)(&packet), sizeof(packet)) != sizeof(packet)) {
            qDebug() << "read error";
            return;
        }
        char * data = nullptr;
        if (packet.dataSize) {
            data = new char[packet.dataSize+1];
            size_t readed = clientConnection->read((char *)data, packet.dataSize);
            if (readed != packet.dataSize) {
                qDebug() << "data read error";
                throw "bad data";
            }
        }
        switch (packet.type) {
        case PT_ITEM_CREATED:
        {
            qDebug() << "item created message";
            qDebug() << "id : " << packet.itemId;
            qDebug() << "text: " << data;
            LogItem *parent = control->findItemById(packet.parentId);
            if (parent) {
                LogItem *newItem = new LogItem(parent, packet.itemId);
                newItem->setText(data);
                parent->addAsChild(newItem);
                control->save();
            }
        }
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
    XmlDB db("/tmp/temp.xml");
    LogControl control(&db);

    todoServer.setControl(&control);
    control.loadData();
    todoServer.start();

    return a.exec();
}

#include "main.moc"
