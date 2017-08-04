#include "settings.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>

Settings::Settings()
{

}

bool Settings::loadFromFile(QString fileName)
{
    QFile input(fileName);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader xmlStream;
    xmlStream.setDevice(&input);

    while (!xmlStream.atEnd() && !xmlStream.hasError())
    {
        QXmlStreamReader::TokenType token = xmlStream.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
//            qDebug() << "token readed: " << xmlStream.name() << xmlStream.text();
            if (xmlStream.name() == "meta") {
                loadMetadata(xmlStream, control);
                continue;
            }

            if (xmlStream.name() == "item") {

            }
        }
    }
    if (xmlStream.hasError())
        qDebug() << "read error: " << xmlStream.errorString();

    if (currentItem && currentItem->getId() > maxId)
        maxId = currentItem->getId();
    LogItem::setNextId(maxId+1);

    input.close();

}

bool Settings::saveToFile(QString fileName)
{
    QXmlStreamWriter xmlWriter;
    QFile output(fileName);
    output.open(QIODevice::WriteOnly);
    xmlWriter.setDevice(&output);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("document");

    xmlWriter.writeStartElement("meta");
    xmlWriter.writeTextElement("name", QString("%1").arg(doc->getName()));
    xmlWriter.writeTextElement("id", QString("%1").arg(doc->getId()));
    xmlWriter.writeEndElement();



    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    output.close();
}

void Settings::setIp(QString newIp)
{

}

void Settings::setPort()
{

}
