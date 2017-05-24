#ifndef CORE_H
#define CORE_H

#include <memory>
#include <map>
#include <vector>

#include <QString>
#include <QObject>

#include "types.h"

class ClientDocument;

enum class ItemState
{
    NOT_PRESENT = 0,
    DOWNLOADING,
    UPLOADING,
    PRESENT
};

class ClientItem
{
    uint64_t id;
    ItemType type;
    ItemState state;

    ClientDocument *control;
    bool modified;
    bool syncedWithServer;

    ClientItem *parent;
    ClientItem *next;
    ClientItem *prev;
    ClientItem *firstChild;

    QString text;
    bool done;

    bool folded;

    static uint64_t nextId; //for debug

    friend class ClientDocument;
    friend class DB;

public:

    static void setNextId(uint64_t id) {if (id > nextId) nextId = id;}

    ClientItem(ClientDocument *control, ClientItem *parent, uint64_t id = 0);
    void switchTo(MoveEvent to);
    void remove();
    void detachFromTree();
    void addAsChild(ClientItem *item, ClientItem *after = nullptr);
    void addAsLastChild(ClientItem *item);

    const QString &getText() {return text;}
    void setText(const QString newText) {text = newText;}
    ClientItem *getParent() const {return parent;}
    ClientItem *getChild() const {return firstChild;}
    ClientItem *getLastChild() const {
        if (!firstChild)
            return nullptr;
        ClientItem *temp = firstChild;
        while(temp->next)
            temp = temp->next;
        return temp;
    }
    ClientItem *getNext() const {return next;}
    ClientItem *getPrev() const {return prev;}

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
    ClientItem *item;
    ClientItem *parent;
    ClientItem *prev;
public:
    DeleteAction(ClientDocument *doc, ClientItem *item);

    void make();
    void revert();
};

class CreateAction: public ClientAction
{
    ClientItem *item;
    ClientItem *parent;
    ClientItem *prev;
public:
    CreateAction(ClientDocument *doc, ClientItem *parent, ClientItem *prev);

    void make();
    void revert();
};

class EditAction: public ClientAction
{
    ClientItem *item;
    QString textBeforeEdit;
    QString textAfterEdit;
public:
    EditAction(ClientDocument *doc, ClientItem *item, QString newText);

    void make();
    void revert();
};

class MoveAction: public ClientAction
{
    ClientItem *item;
    ClientItem *oldParent;
    ClientItem *oldPrev;
    ClientItem *newParent;
    ClientItem *newPrev;
public:
    MoveAction(ClientDocument *doc, ClientItem *item, ClientItem *newParent, ClientItem *newPrev);

    void make();
    void revert();
};

enum DocumentStatus
{
    DS_OPEN = 0,
    DS_LOADING,
    DS_CLOSED
};

class RemoteDB;

class ClientDocument: public QObject
{
    Q_OBJECT

    ClientItem *rootItem;
    RemoteDB *serverDB;
    uint64_t id;

    QString name;
    DocumentType docType;
    DocumentStatus docStatus;

    std::vector<ClientAction *> actionList;
    std::vector<ClientAction *> redoActionList;

    bool modified;

    static uint64_t maxDocId;

    void fillGui(ClientItem *item);
    ClientItem *findItem(ClientItem *parent, uint64_t id);

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
    void setRootItem(ClientItem *root);
    void setServerDB(RemoteDB *db, DocumentType type);
    const RemoteDB *getRemoteDB() const { return serverDB; }
    uint64_t getId() const {return id;}

    ClientItem *findItemById(uint64_t id);
    ClientItem *getNextItemInTree(ClientItem *item);
    ClientItem *getPrevItemInTree(ClientItem *item);

    ClientItem *getRootItem();

    void addItem(ClientItem *item, ClientItem *parent, ClientItem *prev = nullptr);

    void printItemTree();

    DocumentStatus getStatus() const {return docStatus;}
    DocumentType getType() const {return docType;}

    void setModified(bool modified);
    bool getModified() const {return modified;}

public slots:
    void setItemDone(ClientItem *item, bool state);
    void setItemFold(ClientItem *item, bool state);
    void setItemText(ClientItem *item, QString text);
    void undoLastAction();
    void redoAction();
    void switchFocusTo(ClientItem *item, int to);
    void createNewItem(ClientItem *parent, ClientItem *prev);
    void moveItem(ClientItem *item, int direction);
    void removeItem(ClientItem *item);
    void save();

signals:
    void docModifiedChanged(bool modified);
    void docSaved();
    void itemAdded(ClientItem *);
    void itemCreated(ClientItem *);
    void itemStateChanged(ClientItem *);
    void itemTextChanged(ClientItem *);
    void itemModified(ClientItem *);
    void itemDeleted(ClientItem *);
    void itemFocused(ClientItem *);
    void itemDoneChanged(ClientItem *);
    void itemFoldChanged(ClientItem *);
};

#endif // CORE_H
