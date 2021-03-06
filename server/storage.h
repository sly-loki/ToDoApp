#ifndef STORAGE_H
#define STORAGE_H

#include <map>
#include <vector>

#include <QTextStream>
#include <QXmlStreamReader>
#include <QFile>

class LogItem;
class ServerDocument;

typedef std::vector<std::pair<LogItem *, QString>> ItemVector;

class DB: public QObject
{
    Q_OBJECT

public:
    DB();
    virtual void saveItem(LogItem *item, const QString &text);
    virtual void saveDocument(ServerDocument *doc) = 0;
    virtual void loadTree(ServerDocument *control, LogItem *rootItem) = 0;
    virtual ItemVector getFirstLevelItems();
    virtual ItemVector getChildsOf(LogItem *item);
    virtual QString getText(LogItem *item);
    virtual ~DB() {}
signals:
    void loadingDone();

};

class XmlDB: public DB
{
    QXmlStreamReader xmlStream;
    QXmlStreamWriter xmlWriter;
    QFile dbFile;
    bool fileIsOk;
    QString fileName;

    void saveNode(QXmlStreamWriter &stream, LogItem *node);
    void loadNode(QXmlStreamReader &stream, LogItem *node);
    void loadMetadata(QXmlStreamReader &stream, ServerDocument *doc);
public:
    XmlDB(const QString fileName);
    virtual void saveItem(LogItem *item, const QString &text) override;
    virtual void saveDocument(ServerDocument *doc) override;
    virtual void loadTree(ServerDocument *control, LogItem *rootItem) override;
    virtual ItemVector getFirstLevelItems() override;
    virtual ItemVector getChildsOf(LogItem *item) override;
    virtual QString getText(LogItem *item) override;
    virtual ~XmlDB();
};

#endif // STORAGE_H
