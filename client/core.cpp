#include "core.h"
#include <functional>

#include <QDebug>

#include "logappserver.h"

uint64_t ClientItem::nextId = 1;
uint64_t ClientDocument::maxDocId = 0;

ClientItem::ClientItem(ClientDocument *control, ClientItem *parent, uint64_t id)
    : id(id)
    , type(ItemType::TODO)
    , state(ItemState::NOT_PRESENT)
    , control(control)
    , modified(false)
    , syncedWithServer(false)
    , parent(parent)
    , next(nullptr)
    , prev(nullptr)
    , firstChild(nullptr)
    , text("")
    , done(false)
    , folded(false)
{
    if (id == 0) {
        this->id = nextId;
        nextId++;
    } else {
        if (id >= nextId)
            nextId = id+1;
    }
//    if (parent)
//        parent->addAsChild(this);
}

void ClientItem::switchTo(MoveEvent to)
{
    control->switchFocusTo(this, to);
}

void ClientItem::remove()
{
    control->removeItem(this);
}

void ClientItem::detachFromTree()
{
    if (parent && parent->firstChild == this)
        parent->firstChild = this->next;
    if (prev)
        prev->next = this->next;
    if (next)
        next->prev = this->prev;
    prev = next = nullptr;
}

void ClientItem::addAsChild(ClientItem *item, ClientItem *after)
{
    if (after && after->parent != this) {
        qDebug() << "Error: after->parent != this";
        after = nullptr;
    }

    if (after) {
        if (after->next)
            after->next->prev = item;
        item->next = after->next;
        item->prev = after;
        after->next = item;
    } else {
        if (firstChild)
            firstChild->prev = item;
        item->next = firstChild;
        firstChild = item;
    }

    item->parent = this;
}

void ClientItem::addAsLastChild(ClientItem *item)
{
    if (!firstChild) {
        addAsChild(item);
        return;
    }
    ClientItem *temp = getLastChild();
    temp->next = item;
    item->prev = temp;
    item->parent = this;
}

void ClientItem::save()
{
    control->save();
}

void ClientItem::cleanModified()
{
    modified = false;
    ClientItem *temp = getChild();
    while (temp) {
        temp->cleanModified();
        temp = temp->getNext();
    }
}

uint64_t ClientItem::getId() const
{
    return id;
}

void ClientItem::setId(const uint64_t &value)
{
    id = value;
}

//++++++++++++++++++++++++++++++++++++++++++++

DeleteAction::DeleteAction(ClientDocument *doc, ClientItem *item)
    : ClientAction(doc)
    , item(item)
    , parent(item->getParent())
    , prev(item->getPrev())
{
}

void DeleteAction::make()
{
    ClientItem *newFocusedItem = item->getPrev()?item->getPrev():item->getParent();

    item->detachFromTree();

    emit doc->itemDeleted(item);
    emit doc->itemFocused(newFocusedItem);
}

void DeleteAction::revert()
{
    doc->addItem(item, parent, prev);
}

ClientAction::ClientAction(ClientDocument *doc)
    : doc(doc)
{

}

CreateAction::CreateAction(ClientDocument *doc, ClientItem *parent, ClientItem *prev)
    : ClientAction(doc)
    , parent(parent)
    , prev(prev)
{

}

void CreateAction::make()
{
    if (!parent)
        parent = doc->getRootItem();
    ClientItem *item = new ClientItem(doc, parent);
    parent->addAsChild(item, prev);
    emit doc->itemAdded(item);
    this->item = item;
    emit doc->itemFocused(item);
    emit doc->itemCreated(item);
}

void CreateAction::revert()
{
    ClientItem *newFocusedItem = item->getPrev()?item->getPrev():item->getParent();

    item->detachFromTree();

    emit doc->itemDeleted(item);
    emit doc->itemFocused(newFocusedItem);
}

EditAction::EditAction(ClientDocument *doc, ClientItem *item, QString newText)
    : ClientAction(doc)
    , item(item)
    , textBeforeEdit(item->getText())
    , textAfterEdit(newText)
{

}

void EditAction::make()
{
    item->setText(textAfterEdit);
    emit doc->itemTextChanged(item);
}

void EditAction::revert()
{
    item->setText(textBeforeEdit);
    emit doc->itemTextChanged(item);
    emit doc->itemFocused(item);
}

MoveAction::MoveAction(ClientDocument *doc, ClientItem *item, ClientItem *newParent, ClientItem *newPrev)
    : ClientAction(doc)
    , item(item)
    , oldParent(item->getParent())
    , oldPrev(item->getPrev())
    , newParent(newParent)
    , newPrev(newPrev)
{

}

void MoveAction::make()
{
    item->detachFromTree();
    newParent->addAsChild(item, newPrev);

    emit doc->itemModified(item);
}

void MoveAction::revert()
{
    item->detachFromTree();
    oldParent->addAsChild(item, oldPrev);

    emit doc->itemModified(item);
}


//######################################

void ClientDocument::fillGui(ClientItem *item)
{
    if (item->getParent() != nullptr)
        emit itemAdded(item);
        ClientItem *child = item->getChild();
    while (child) {
        child->control = this;
        fillGui(child);
        child = child->getNext();
    }
}

ClientItem *ClientDocument::findItem(ClientItem *parent, uint64_t id)
{
    if (parent->getId() == id)
        return parent;
    ClientItem *child = parent->getChild();
    while(child) {
        ClientItem *temp = findItem(child, id);
        if (temp)
            return temp;
        child = child->getNext();
    }
    return nullptr;
}

void ClientDocument::doAction(ClientAction *action)
{
    action->make();
    actionList.push_back(action);
    redoActionList.clear();
    setModified(true);
}

void ClientDocument::setStatus(DocumentStatus status)
{
    if (status != docStatus)
        docStatus = status;
}

void ClientDocument::onLoadingDone()
{
    if (!rootItem->getChild()) {
        createNewItem(rootItem, nullptr);
    }
    else {
//        fillGui(rootItem);
    }
    printItemTree();
    docStatus = DS_OPEN;
}

ClientDocument::ClientDocument(QString name, uint64_t id)
    : rootItem(new ClientItem(this, nullptr))
    , serverDB(nullptr)
    , id(id)
    , name(name)
    , docType(DT_LOCAL)
    , docStatus(DS_CLOSED)
    , modified(false)
{
    rootItem->setId(0);
    //rootItem = std::unique_ptr<LogItem>(new LogItem(this));
    if (id > maxDocId)
        maxDocId = id;
}

QString ClientDocument::getName() const
{
    return name;
}

void ClientDocument::setName(QString name)
{
    this->name = name;
}

void ClientDocument::loadData()
{
    docStatus = DS_LOADING;
    serverDB->start();
}

void ClientDocument::setRootItem(ClientItem *root)
{
    delete rootItem;
    rootItem = root;
    if (!rootItem->getChild()) {
        createNewItem(rootItem, nullptr);
    }
    else {
        fillGui(rootItem);
    }
}

void ClientDocument::setServerDB(RemoteDB *db, DocumentType type)
{
    if (serverDB) {
        qDebug() << "server db already set";
        return;
    }
    serverDB = db;
    docType = type;
}

ClientItem *ClientDocument::findItemById(uint64_t id)
{
    return findItem(rootItem, id);
}

ClientItem *ClientDocument::getNextItemInTree(ClientItem *item)
{
    if (item->getChild() && !item->isFolded())
        return item->getChild();
    else if (item->getNext())
        return item->getNext();
    else {
        ClientItem *temp = item->getParent();
        while (temp) {
            if (temp->getNext()) {
                return temp->getNext();
            }
            temp = temp->getParent();
        }
    }
    return nullptr;
}

ClientItem *ClientDocument::getPrevItemInTree(ClientItem *item)
{
    if (item == rootItem)
        return rootItem;
    if (item->prev) {
        ClientItem *temp = item->prev->getLastChild();
        if (temp && !item->prev->isFolded()) {
            while (temp->getChild())
                temp = temp->getLastChild();
            return temp;
        }
        else {
            return item->getPrev();
        }
    }
    else {
        ClientItem *parent = item->getParent();
        return (parent != rootItem)?parent:item;
    }
}

ClientItem *ClientDocument::getRootItem()
{
    return rootItem;
}

void ClientDocument::addItem(ClientItem *item, ClientItem *parent, ClientItem *prev)
{
    parent->addAsChild(item, prev);
    emit itemAdded(item);
}

void ClientDocument::removeItem(ClientItem *item)
{
    if (item->getChild()) {
        return;
    }
    DeleteAction *action = new DeleteAction(this, item);
    doAction(action);
}

void ClientDocument::switchFocusTo(ClientItem *item, int to)
{
    static const int PAGESTEP = 5;
    ClientItem *nextItem = item;
    switch (to) {
    case UP:
        nextItem = getPrevItemInTree(item);
        break;
    case DOWN:
        nextItem = getNextItemInTree(item);
        break;
    case LEFT:
        if (item->getParent() && item->getParent() != rootItem)
            nextItem = item->getParent();
        break;
    case PAGE_UP:
        for (int i = 0; i < PAGESTEP; i++)
            nextItem = getPrevItemInTree(nextItem);
        break;
    case PAGE_DOWN:
        for (int i = 0; i < PAGESTEP; i++) {
            ClientItem *temp;
            temp = getNextItemInTree(nextItem);
            if (temp)
                nextItem = temp;
            else
                break;
        }
        break;
    case TO_BEGIN:
        nextItem = rootItem->getChild();
        break;
    case TO_END:
        nextItem = rootItem->getLastChild();
        while(nextItem->getLastChild())
            nextItem = nextItem->getLastChild();
        break;
    default:
        break;
    }
    emit itemFocused(nextItem);
}

void ClientDocument::createNewItem(ClientItem *parent, ClientItem *prev)
{
    CreateAction *action = new CreateAction(this, parent, prev);
    doAction(action);
}

void ClientDocument::moveItem(ClientItem *item, int direction)
{
    ClientItem *newParent;
    ClientItem *newPrev = nullptr;

    switch(direction) {
    case UP:
        if (!item->getPrev())
            return;
        newPrev = item->getPrev()->getPrev();
        newParent = item->getParent();
        break;
    case DOWN:
        if (!item->getNext())
            return;
        newPrev = item->getNext();
        newParent = item->getParent();
        break;
    case LEFT:
        if(item->getParent() == rootItem)
            return;
        newParent = item->getParent()->getParent();
        newPrev = item->getParent();
        break;
    case RIGHT:
        if (!item->getPrev())
            return;
        newParent = item->getPrev();
        newPrev = item->getPrev()->getLastChild();
        break;
    default:
        break;
    }

    MoveAction *action = new MoveAction(this, item, newParent, newPrev);
    doAction(action);
}

void ClientDocument::save()
{
    emit docSaved();
    setModified(false);
}

void ClientDocument::printItemTree()
{
    std::function<void (ClientItem *, QString)> f = [&f](ClientItem *item, QString intents) {
        qDebug()  << intents << item->getId() << " : " << item->getText() << " " << ((item->getParent())?(item->getParent()->getId()):-1);
        ClientItem *child = item->getChild();
        while (child) {
            f(child, intents + "    ");
            child = child->getNext();
        }
    };
    f(rootItem, QString(""));
}

void ClientDocument::setModified(bool modified)
{
    if (modified != this->modified) {
        this->modified = modified;
        emit docModifiedChanged(this->modified);
    }
}

void ClientDocument::setItemDone(ClientItem *item, bool state)
{
    if (item->isDone() == state)
        return;

    item->setDone(state);
    emit itemDoneChanged(item);
    if (state == true) {
        ClientItem *child = item->getChild();
        while (child) {
            setItemDone(child, state);
            child = child->getNext();
        }
    }
}

void ClientDocument::setItemFold(ClientItem *item, bool state)
{
    if (item->isFolded() == state)
        return;
    item->setFolded(state);
    emit itemFoldChanged(item);
}

void ClientDocument::setItemText(ClientItem *item, QString text)
{
    EditAction *action = new EditAction(this, item, text);
    doAction(action);
}

void ClientDocument::undoLastAction()
{
    if (actionList.size() == 0)
        return;

    ClientAction *action = actionList.back();
    actionList.pop_back();
    action->revert();
    redoActionList.push_back(action);
    setModified(true);
}

void ClientDocument::redoAction()
{
    if (redoActionList.empty())
        return;

    ClientAction *action = redoActionList.back();
    redoActionList.pop_back();
    action->make();
    actionList.push_back(action);
    setModified(true);
}

