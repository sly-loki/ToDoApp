#ifndef CORE_H
#define CORE_H

#include <memory>
#include <map>
#include <vector>

#include "storage.h"

class LogControl;

enum MoveEvent {
    ME_UP,
    ME_DOWN,
    ME_LEFT
};

class LogItem
{
    uint64_t id;

    LogControl *control;
    bool modified;
    bool syncedWithServer;

    LogItem *parent;
    LogItem *next;
    LogItem *prev;
    LogItem *firstChild;

    QString text;
    bool done;

    static uint64_t nextId; //for debug

    friend class LogControl;
    friend class DB;

public:

    static void setNextId(uint64_t id) {if (id > nextId) nextId = id;}

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
    void addAsChild(LogItem *item, LogItem *after = nullptr);
    void addAsLastChild(LogItem *item);

    const QString &getText() {return text;}
    void setText(const QString newText) {text = newText;}
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

    bool isSynced() {return syncedWithServer;}
    void setSynced(bool status) {syncedWithServer = status;}

    bool isDone() {return done;}
    void setDone(bool state) {this->done = state;}

    uint64_t getId() const;
    void setId(const uint64_t &value);
};

class LogControl: public QObject
{
    Q_OBJECT

    LogItem *rootItem;
    DB *db;

    void fillGui(LogItem *item);
    LogItem *findItem(LogItem *parent, uint64_t id);
public:
    LogControl(DB* db);
    void loadData();
    void setRootItem(LogItem *root);

    LogItem *findItemById(uint64_t id);

    LogItem *getRootItem();
    LogItem *createNewChild(LogItem *parent);
    LogItem *createNewSibling(LogItem *item);
    void shiftRight(LogItem *item);
    void shiftLeft(LogItem *item);
    void shiftUp(LogItem *item);
    void shiftDown(LogItem *item);
    void removeItem(LogItem *item);
    void switchTo(LogItem *item, MoveEvent to);
    void save();

    void printItemTree();

public slots:
    void setItemDone(LogItem *item, bool state);

signals:
    void itemAdded(LogItem *);
    void itemModified(LogItem *);
    void itemDeleted(LogItem *);
    void itemFocused(LogItem *);
    void itemDoneChanged(LogItem *);
};

class LogDocument
{
public:
    LogDocument();

    LogItem *getRootItem();
};

#endif // CORE_H
