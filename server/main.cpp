#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include <memory>
#include <functional>
#include <cassert>
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

    void sendPacket(NetworkHeader *header, const void *data)
    {
        assert(header);
        if (header->dataSize)
            assert(data != nullptr);

        qDebug() << "send package";

        clientConnection->write((char *)header, sizeof(NetworkHeader));
        if (header->dataSize)
            clientConnection->write((char *)data, header->dataSize);
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
        while (true) {
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
                LogItem *newItem = new LogItem(control, parent, packet.itemId);
                newItem->setText(data);
                parent->addAsChild(newItem);
                control->save();
            }
        }
            break;
        case PT_GET_ITEM:
        {
            qDebug() << "get item: " << packet.itemId;
            LogItem *item = control->findItemById(packet.itemId);
            if (item) {
                packet.dataSize = item->getText().size();
                packet.parentId = (item->getParent())?item->getParent()->getId():0;
                sendPacket(&packet, item->getText().toStdString().c_str());
            } else {
                // error processing here
            }
        }
            break;
        case PT_GET_ALL_ITEMS:
        {
            qDebug() << "request for all item";
            std::vector<uint64_t> ids;
            LogItem *root = control->getRootItem();
            std::function<void (LogItem *, std::vector<uint64_t> &)> f = [&f](LogItem *item, std::vector<uint64_t> &ids){
                ids.push_back(item->getId());
                LogItem *child = item->getChild();
                while (child) {
                    f(child, ids);
                    child = child->getNext();
                }
            };
            f(root, ids);
            packet.dataSize = ids.size() * sizeof(uint64_t);
            sendPacket(&packet, (const void *)ids.data());
            break;
        }
        default:
            qDebug() << "error here";
            break;
        }
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
    XmlDB db("/tmp/test.xml");
    LogControl control(&db);

    todoServer.setControl(&control);
    control.loadData();
    todoServer.start();

    return a.exec();
}

#include "main.moc"
