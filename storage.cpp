#include "storage.h"
#include "logtextedit.h"

#include <QDebug>

DB::DB()
{

}

void DB::saveItem(LogItem *item, const QString &text)
{

}

void DB::saveTree(LogItem *rootItem)
{

}

ItemVector DB::getFirstLevelItems()
{

}

ItemVector DB::getChildsOf(LogItem *item)
{

}

QString DB::getText(LogItem *item)
{

}


void XmlDB::saveNode(QXmlStreamWriter &stream, LogItem *node)
{
    stream.writeStartElement("item");
    stream.writeTextElement("id", QString("%1").arg(node->getId()));
    stream.writeTextElement("parent", QString("%1").arg(node->getParent()->getId()));
    stream.writeTextElement("text", node->getText());
    stream.writeEndElement();
    LogItem *child = node->getChild();
    while(child) {
        saveNode(stream, child);
        child = child->getNext();
    }
}

void XmlDB::loadNode(QXmlStreamReader &stream, LogItem *node)
{

}

XmlDB::XmlDB(const QString fileName)
    : fileIsOk(false)
{
    dbFile.setFileName(fileName);
    if (dbFile.open(QIODevice::ReadOnly)) {
        xmlStream.setDevice(&dbFile);
        fileIsOk = true;
    }
}

void XmlDB::saveItem(LogItem *item, const QString &text)
{
//    item->getId();
}

#define TEST_FILE_NAME "/tmp/test.xml"

void XmlDB::saveTree(LogItem *rootItem)
{
//    if (fileIsOk)
//        return;
    QFile output(TEST_FILE_NAME);
    output.open(QIODevice::WriteOnly);
    xmlWriter.setDevice(&output);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("document");
    LogItem *item = rootItem->getChild();
    while(item) {
        saveNode(xmlWriter, item);
        item = item->getNext();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    output.close();
}

void XmlDB::loadTree(LogItem *rootItem)
{
    QFile input(TEST_FILE_NAME);
    input.open(QIODevice::ReadOnly);
    xmlStream.setDevice(&input);

    std::map<uint64_t, LogItem*> items;
    items[0] = rootItem;

    LogItem *currentItem = nullptr;
    static const uint64_t TEMPORARY_ID = 0;
    while (!xmlStream.atEnd() && !xmlStream.hasError())
    {
        QXmlStreamReader::TokenType token = xmlStream.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            qDebug() << "token readed: " << xmlStream.name() << xmlStream.text();
            if (xmlStream.name() == "item") {

                if (currentItem) {

                    if (currentItem->getId() == TEMPORARY_ID) {

                        qDebug() << "Error: item without id!!";
                    } else {

                        if (items.find(currentItem->getId()) != items.end()) {
                            qDebug() << "Error: dublicate id!!";
                        } else {
                            items[currentItem->getId()] = currentItem;
                            currentItem = new LogItem(nullptr, nullptr, TEMPORARY_ID);
                            qDebug() << "create new item";
                        }
                    }
                } else {
                    currentItem = new LogItem(nullptr, nullptr, TEMPORARY_ID);
                }
            } else {
                if (!currentItem)
                    continue;
                QString type = xmlStream.name().toString();
                qDebug() << "type: " << type;
                xmlStream.readNext();
                if (type == "id") {
                    uint64_t id = xmlStream.text().toUInt();
                    qDebug() << "setId: " << id;
                    currentItem->setId(id);
                } else if (type == "parent") {
                    uint64_t pid = xmlStream.text().toUInt();
                    if (items.find(pid) != items.end()) {
                        LogItem *parent = items[pid];
                        parent->addAsLastChild(currentItem);
                    }
                } else if (type == "text") {
                    currentItem->setText(xmlStream.text().toString());
                }
            }
        }
    }
    if (xmlStream.hasError())
        qDebug() << "read error: " << xmlStream.errorString();

    input.close();
}

ItemVector XmlDB::getFirstLevelItems()
{
    ItemVector result;
    return result;
}

ItemVector XmlDB::getChildsOf(LogItem *item)
{
    ItemVector result;
    return result;
}

QString XmlDB::getText(LogItem *item)
{
    return "";
}

XmlDB::~XmlDB()
{

}

