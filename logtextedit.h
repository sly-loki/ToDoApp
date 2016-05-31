#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QLayout>
#include <memory>
#include <map>
#include <vector>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QFile>

class LogControl;
class LogItem;

class GuiControl
{
    QWidget *rootWidget;
    std::map<LogItem*, QWidget*> guiItemsMap;
public:
    GuiControl(QWidget *rootWidget);
    void createNewSiblingElement(LogItem *item);
    void createNewChildElement(LogItem *item);
    void switchFocusTo(LogItem *item);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);
};

typedef std::vector<std::pair<LogItem *, QString>> ItemVector;

class DB
{
public:
    DB();
    virtual void saveItem(LogItem *item, const QString &text);
    virtual void saveTree(LogItem *rootItem) = 0;
    virtual void loadTree(LogItem *rootItem) = 0;
    virtual ItemVector getFirstLevelItems();
    virtual ItemVector getChildsOf(LogItem *item);
    virtual QString getText(LogItem *item);
    virtual ~DB() {}

};

class XmlDB: public DB
{
    QXmlStreamReader xmlStream;
    QXmlStreamWriter xmlWriter;
    QFile dbFile;
    bool fileIsOk;

    void saveNode(QXmlStreamWriter &stream, LogItem *node);
public:
    XmlDB(const QString fileName);
    virtual void saveItem(LogItem *item, const QString &text) override;
    virtual void saveTree(LogItem *rootItem);
    virtual void loadTree(LogItem *rootItem);
    virtual ItemVector getFirstLevelItems() override;
    virtual ItemVector getChildsOf(LogItem *item) override;
    virtual QString getText(LogItem *item) override;
    virtual ~XmlDB();
};

enum MoveEvent {
    ME_UP,
    ME_DOWN,
    ME_LEFT
};

class LogItem
{
    uint64_t id;
    LogControl *control;
    LogItem *parent;
    LogItem *next;
    LogItem *prev;
    LogItem *firstChild;

    QWidget *guiElement;
    bool modified;

    static uint64_t nextId;

    friend class LogControl;
    friend class DB;

public:

    LogItem(LogControl *control, LogItem *parent, uint64_t id = 0);
    void addNewChild();
    void addNewSibling();
    void shiftRight();
    void shiftLeft();
    void shiftUp();
    void shiftDown();
    void switchTo(MoveEvent to);
    void remove();
    void detachFromTree();
    void addAsChild(LogItem *item);
    void addAsLastChild(LogItem *item);

    void setGui(QWidget *widget);
    LogItem *getParent() {return parent;}
    LogItem *getChild() {return firstChild;}
    LogItem *getLastChild() {
        if (!firstChild)
            return nullptr;
        LogItem *temp = firstChild;
        while(temp->next)
            temp = temp->next;
        return temp;
    }
    LogItem *getNext() {return next;}
    LogItem *getPrev() {return prev;}

    void save();

    bool isModified() {return modified;}
    void setModified(bool status) {modified = status;}

    uint64_t getId() const;
    void setId(const uint64_t &value);
};

class LogControl
{
    LogItem *rootItem;
    GuiControl *guiControl;
    DB *db;
public:
    LogControl(GuiControl *gui, DB* db);
    LogItem *createNewChild(LogItem *parent);
    LogItem *createNewSibling(LogItem *item);
    void shiftRight(LogItem *item);
    void shiftLeft(LogItem *item);
    void shiftUp(LogItem *item);
    void shiftDown(LogItem *item);
    void removeItem(LogItem *item);
    void switchTo(LogItem *item, MoveEvent to);
    void save();
};

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    LogItem *item;
public:
    LogTextEdit(LogItem *item, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e);
};

#endif // LOGTEXTEDIT_H
