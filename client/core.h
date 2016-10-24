#ifndef CORE_H
#define CORE_H

#include <memory>
#include <map>
#include <vector>

#include <QString>
#include <QObject>

class ClientDocument;

enum MoveEvent {


    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    TO_BEGIN,
    TO_END
};

enum class ItemType
{
    TODO = 0,
    LOG
};

enum class ItemState
{
    NOT_PRESENT = 0,
    DOWNLOADING,
    UPLOADING,
    PRESENT
};

class LogItem
{
    uint64_t id;
    ItemType type;
    ItemState state;

    ClientDocument *control;
    bool modified;
    bool syncedWithServer;

    LogItem *parent;
    LogItem *next;
    LogItem *prev;
    LogItem *firstChild;

    QString text;
    bool done;

    bool folded;

    static uint64_t nextId; //for debug

    friend class ClientDocument;
    friend class DB;

public:

    static void setNextId(uint64_t id) {if (id > nextId) nextId = id;}

    LogItem(ClientDocument *control, LogItem *parent, uint64_t id = 0);
    void switchTo(MoveEvent to);
    void remove();
    void detachFromTree();
    void addAsChild(LogItem *item, LogItem *after = nullptr);
    void addAsLastChild(LogItem *item);

    const QString &getText() {return text;}
    void setText(const QString newText) {text = newText;}
    LogItem *getParent() const {return parent;}
    LogItem *getChild() const {return firstChild;}
    LogItem *getLastChild() const {
        if (!firstChild)
            return nullptr;
        LogItem *temp = firstChild;
        while(temp->next)
            temp = temp->next;
        return temp;
    }
    LogItem *getNext() const {return next;}
    LogItem *getPrev() const {return prev;}

    void save();

    bool isModified() const {return modified;}
    void setModified(bool status) {modified = status;}
    void cleanModified();

    bool isSynced() const {return syncedWithServer;}
    void setSynced(bool status) {syncedWithServer = status;}

    bool isDone() const {return done;}
    void setDone(bool state) {this->done = state;}

    void setState(ItemState state) {this->state = state;}
    ItemState getState() const {return state;}

    uint64_t getId() const;
    void setId(const uint64_t &value);

    void setFolded(bool folded) {this->folded = folded;}
    bool isFolded() const {return folded;}

    void setType(ItemType type) {this->type = type;}
    ItemType getType() const {return type;}
};

class ClientAction
{
protected:
    ClientDocument *doc;
public:
    ClientAction(ClientDocument *doc);
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
    DeleteAction(ClientDocument *doc, LogItem *item);

    void make();
    void revert();
};

class CreateAction: public ClientAction
{
    LogItem *item;
    LogItem *parent;
    LogItem *prev;
public:
    CreateAction(ClientDocument *doc, LogItem *parent, LogItem *prev);

    void make();
    void revert();
};

class EditAction: public ClientAction
{
    LogItem *item;
    QString textBeforeEdit;
    QString textAfterEdit;
public:
    EditAction(ClientDocument *doc, LogItem *item, QString newText);

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
    MoveAction(ClientDocument *doc, LogItem *item, LogItem *newParent, LogItem *newPrev);

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

class RemoteDB;

class ClientDocument: public QObject
{
    Q_OBJECT

    LogItem *rootItem;
    RemoteDB *serverDB;
    uint64_t id;

    QString name;
    DocumentType docType;
    DocumentStatus docStatus;

    std::vector<ClientAction *> actionList;
    std::vector<ClientAction *> redoActionList;

    bool modified;

    static uint64_t maxDocId;

    void fillGui(LogItem *item);
    LogItem *findItem(LogItem *parent, uint64_t id);

    void doAction(ClientAction *action);
    void setStatus(DocumentStatus status);

protected slots:
    void onLoadingDone();

public:
    static const int ROOT_ITEM_ID = 0;

    static uint64_t getNextDocId() {return ++maxDocId;}

    ClientDocument(QString name, uint64_t id);

    QString getName() const;
    void setName(QString name);
    void loadData();
    void setRootItem(LogItem *root);
    void setServerDB(RemoteDB *db, DocumentType type);
    uint64_t getId() const {return id;}

    LogItem *findItemById(uint64_t id);
    LogItem *getNextItemInTree(LogItem *item);
    LogItem *getPrevItemInTree(LogItem *item);

    LogItem *getRootItem();

    void addItem(LogItem *item, LogItem *parent, LogItem *prev = nullptr);

    void printItemTree();

    DocumentStatus getStatus() const {return docStatus;}
    DocumentType getType() const {return docType;}

    void setModified(bool modified);
    bool getModified() const {return modified;}

public slots:
    void setItemDone(LogItem *item, bool state);
    void setItemFold(LogItem *item, bool state);
    void setItemText(LogItem *item, QString text);
    void undoLastAction();
    void redoAction();
    void switchFocusTo(LogItem *item, int to);
    void createNewItem(LogItem *parent, LogItem *prev);
    void moveItem(LogItem *item, int direction);
    void removeItem(LogItem *item);
    void save();

signals:
    void docModifiedChanged(bool modified);
    void itemAdded(LogItem *);
    void itemCreated(LogItem *);
    void itemStateChanged(LogItem *);
    void itemTextChanged(LogItem *);
    void itemModified(LogItem *);
    void itemDeleted(LogItem *);
    void itemFocused(LogItem *);
    void itemDoneChanged(LogItem *);
    void itemFoldChanged(LogItem *);
};

#endif // CORE_H
