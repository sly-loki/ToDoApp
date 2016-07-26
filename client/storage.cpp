#include "storage.h"
#include "core.h"

#include <QDebug>

DB::DB()
{

}

void DB::saveItem(LogItem *item, const QString &text)
{
    Q_UNUSED(item);
    Q_UNUSED(text);
}

void DB::saveTree(LogItem *rootItem)
{
    Q_UNUSED(rootItem);
}

ItemVector DB::getFirstLevelItems()
{
    return ItemVector();
}

ItemVector DB::getChildsOf(LogItem *item)
{
    Q_UNUSED(item);
    return ItemVector();
}

QString DB::getText(LogItem *item)
{
    Q_UNUSED(item);
    return "";
}


void XmlDB::saveNode(QXmlStreamWriter &stream, LogItem *node)
{
    stream.writeStartElement("item");
    stream.writeTextElement("id", QString("%1").arg(node->getId()));
    stream.writeTextElement("parent", QString("%1").arg(node->getParent()->getId()));
    stream.writeTextElement("text", node->getText());
    stream.writeTextElement("done", QString("%1").arg((node->isDone())?1:0));
    stream.writeTextElement("sync", QString("%1").arg((node->isDone())?1:0));
    stream.writeTextElement("folded", QString("%1").arg((node->isChildrenHided())?1:0));
    stream.writeEndElement();
    LogItem *child = node->getChild();
    while(child) {
        saveNode(stream, child);
        child = child->getNext();
    }
}

void XmlDB::loadNode(QXmlStreamReader &stream, LogItem *node)
{
    Q_UNUSED(stream);
    Q_UNUSED(node);
}

XmlDB::XmlDB(const QString fileName)
    : DB()
    , fileIsOk(false)
    , fileName(fileName)
{
    dbFile.setFileName(fileName);
    if (dbFile.open(QIODevice::ReadOnly)) {
        xmlStream.setDevice(&dbFile);
        fileIsOk = true;
    }
}

void XmlDB::saveItem(LogItem *item, const QString &text)
{
    Q_UNUSED(item);
    Q_UNUSED(text);
}

void XmlDB::saveTree(LogItem *rootItem)
{
//    if (fileIsOk)
//        return;
    QFile output(fileName);
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

void XmlDB::loadTree(LogControl *control, LogItem *rootItem)
{
    QFile input(fileName);
    input.open(QIODevice::ReadOnly);
    xmlStream.setDevice(&input);
    uint64_t maxId = 0;

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
//            qDebug() << "token readed: " << xmlStream.name() << xmlStream.text();
            if (xmlStream.name() == "item") {

                if (currentItem) {

                    if (currentItem->getId() == TEMPORARY_ID) {
                        qDebug() << "Error: item without id!!";
                        throw "ha";
                    } else {

                        if (items.find(currentItem->getId()) != items.end()) {
                            qDebug() << "Error: dublicate id: " << currentItem->getId();
                            throw "Error: dublicate id!!";
                        } else {
                            if (currentItem->getId() > maxId)
                                maxId = currentItem->getId();
                            items[currentItem->getId()] = currentItem;
                            currentItem = new LogItem(control, nullptr, TEMPORARY_ID);
                        }
                    }
                } else {
                    currentItem = new LogItem(control, nullptr, TEMPORARY_ID);
                }
            } else {
                if (!currentItem)
                    continue;
                QString type = xmlStream.name().toString();
//                qDebug() << "type: " << type;
                xmlStream.readNext();
                if (type == "id") {
                    uint64_t id = xmlStream.text().toUInt();
//                    qDebug() << "setId: " << id;
                    currentItem->setId(id);
                } else if (type == "parent") {
                    uint64_t pid = xmlStream.text().toUInt();
                    if (items.find(pid) != items.end()) {
                        LogItem *parent = items[pid];
                        parent->addAsLastChild(currentItem);
                    }
                } else if (type == "text") {
                    currentItem->setText(xmlStream.text().toString());
                } else if (type == "done") {
                    currentItem->setDone(xmlStream.text() == "1");
                } else if (type == "sync") {
                    currentItem->setSynced(xmlStream.text() == "1");
                } else if (type == "folded") {
                    currentItem->setChildrenHided(xmlStream.text() == "1");
                }
            }
        }
    }
    if (xmlStream.hasError())
        qDebug() << "read error: " << xmlStream.errorString();

    if (currentItem && currentItem->getId() > maxId)
        maxId = currentItem->getId();
    LogItem::setNextId(maxId+1);

    input.close();
    emit loadingDone();
}

ItemVector XmlDB::getFirstLevelItems()
{
    ItemVector result;
    return result;
}

ItemVector XmlDB::getChildsOf(LogItem *item)
{
    Q_UNUSED(item);
    ItemVector result;
    return result;
}

QString XmlDB::getText(LogItem *item)
{
    Q_UNUSED(item);
    return "";
}

XmlDB::~XmlDB()
{

}
