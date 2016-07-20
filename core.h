#ifndef CORE_H
#define CORE_H

#include <memory>
#include <map>
#include <vector>

#include "storage.h"

class LogControl;

enum MoveEvent {
    ME_UP = 0,
    ME_DOWN,
    ME_LEFT,
    ME_RIGHT,
    ME_PAGE_UP,
    ME_PAGE_DOWN,
    ME_TO_BEGIN,
    ME_TO_END
};

enum ItemType
{
    IT_TODO = 0,
    IT_LOG
};

enum ItemStatus
{
    IS_NOT_PRESENT = 0,
    IS_DOWNLOADING,
    IS_UPLOADING,
    IS_PRESENT
};

class LogItem
{
    uint64_t id;
    ItemType type;
    ItemStatus status;

    LogControl *control;
    bool modified;
    bool syncedWithServer;

    LogItem *parent;
    LogItem *next;
    LogItem *prev;
    LogItem *firstChild;

    QString text;
    bool done;

    bool childrenHided;

    static uint64_t nextId; //for debug

    friend class LogControl;
    friend class DB;

public:

    static void setNextId(uint64_t id) {if (id > nextId) nextId = id;}

    LogItem(LogControl *control, LogItem *parent, uint64_t id = 0);
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

    void setChildrenHided(bool hide) {childrenHided = hide;}
    bool isChildrenHided() {return childrenHided;}
};

class ClientAction
{
protected:
    LogControl *doc;
public:
    ClientAction(LogControl *doc);
    virtual void make() = 0;
    virtual void revert() = 0;
//    virtual void finalise() = 0;

    virtual ~ClientAction() {}
};

class DeleteAction : public ClientAction
{
    LogItem *item;
    LogItem *parent;
    LogItem *prev;
public:
    DeleteAction(LogControl *doc, LogItem *item);

    void make();
    void revert();
};

class CreateAction: public ClientAction
{
    LogItem *item;
    LogItem *parent;
    LogItem *prev;
public:
    CreateAction(LogControl *doc, LogItem *parent, LogItem *prev);

    void make();
    void revert();
};

class EditAction: public ClientAction
{
    LogItem *item;
    QString textBeforeEdit;
    QString textAfterEdit;
public:
    EditAction(LogControl *doc, LogItem *item, QString newText);

    void make();
    void revert();
};

class MoveAction: public ClientAction
{
    LogItem *item;
    LogItem *oldParent;
    LogItem *oldPrev;
    LogItem *newParent;
    LogItem *newPrev;
public:
    MoveAction(LogControl *doc, LogItem *item, LogItem *newParent, LogItem *newPrev);

    void make();
    void revert();
};

enum DocumentStatus
{
    DS_OPEN = 0,
    DS_LOADING,
    DS_CLOSED
};

enum DocumentType
{
    DT_LOCAL = 0,
    DT_REMOTE,
    DT_CACHED
};

class LogControl: public QObject
{
    Q_OBJECT

    LogItem *rootItem;
    DB *db;
    QString name;
    DocumentType docType;
    DocumentStatus docStatus;

    void fillGui(LogItem *item);
    LogItem *findItem(LogItem *parent, uint64_t id);

    std::vector<ClientAction *> actionList;
    std::vector<ClientAction *> redoActionList;

    void doAction(ClientAction *action);
    void setStatus(DocumentStatus status);

protected slots:
    void onLoadingDone();

public:
    LogControl(DB* db, QString name);

    QString getName();
    void loadData();
    void setRootItem(LogItem *root);

    LogItem *findItemById(uint64_t id);
    LogItem *getNextItemInTree(LogItem *item);
    LogItem *getPrevItemInTree(LogItem *item);

    LogItem *getRootItem();

    void addItem(LogItem *item, LogItem *parent, LogItem *prev = nullptr);

    void printItemTree();

    DocumentStatus getStatus() {return docStatus;}

public slots:
    void setItemDone(LogItem *item, bool state);
    void setItemText(LogItem *item, QString text);
    void undoLastAction();
    void redoAction();
    void switchFocusTo(LogItem *item, int to);
    void createNewItem(LogItem *parent, LogItem *prev);
    void moveItem(LogItem *item, int direction);
    void removeItem(LogItem *item);
    void save();

signals:
    void itemAdded(LogItem *);
    void itemStatusChanged(LogItem *);
    void itemTextChanged(LogItem *);
    void itemModified(LogItem *);
    void itemDeleted(LogItem *);
    void itemFocused(LogItem *);
    void itemDoneChanged(LogItem *);
};

#endif // CORE_H
