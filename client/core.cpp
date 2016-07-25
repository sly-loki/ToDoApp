#include "core.h"
#include <functional>

#include <QDebug>

uint64_t LogItem::nextId = 1;

LogItem::LogItem(LogControl *control, LogItem *parent, uint64_t id)
    : id(id)
    , type(IT_TODO)
    , status(IS_NOT_PRESENT)
    , control(control)
    , modified(false)
    , syncedWithServer(false)
    , parent(parent)
    , next(nullptr)
    , prev(nullptr)
    , firstChild(nullptr)
    , text("")
    , done(false)
    , childrenHided(false)
{
    if (id == 0) {
        this->id = nextId;
        nextId++;
    }
//    if (parent)
//        parent->addAsChild(this);
}

void LogItem::switchTo(MoveEvent to)
{
    control->switchFocusTo(this, to);
}

void LogItem::remove()
{
    control->removeItem(this);
}

void LogItem::detachFromTree()
{
    if (parent && parent->firstChild == this)
        parent->firstChild = this->next;
    if (prev)
        prev->next = this->next;
    if (next)
        next->prev = this->prev;
    prev = next = nullptr;
}

void LogItem::addAsChild(LogItem *item, LogItem *after)
{
    if (after && after->parent != this) {
        qDebug() << "Error: after->parent != this";
        return;
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

void LogItem::addAsLastChild(LogItem *item)
{
    if (!firstChild) {
        addAsChild(item);
        return;
    }
    LogItem *temp = getLastChild();
    temp->next = item;
    item->prev = temp;
    item->parent = this;
}

void LogItem::save()
{
    control->save();
}

uint64_t LogItem::getId() const
{
    return id;
}

void LogItem::setId(const uint64_t &value)
{
    id = value;
}

//++++++++++++++++++++++++++++++++++++++++++++

DeleteAction::DeleteAction(LogControl *doc, LogItem *item)
    : ClientAction(doc)
    , item(item)
    , parent(item->getParent())
    , prev(item->getPrev())
{
}

void DeleteAction::make()
{
    LogItem *newFocusedItem = item->getPrev()?item->getPrev():item->getParent();

    item->detachFromTree();

    emit doc->itemDeleted(item);
    emit doc->itemFocused(newFocusedItem);
}

void DeleteAction::revert()
{
    doc->addItem(item, parent, prev);
}

ClientAction::ClientAction(LogControl *doc)
    : doc(doc)
{

}

CreateAction::CreateAction(LogControl *doc, LogItem *parent, LogItem *prev)
    : ClientAction(doc)
    , parent(parent)
    , prev(prev)
{

}

void CreateAction::make()
{
    if (!parent)
        parent = doc->getRootItem();
    LogItem *item = new LogItem(doc, parent);
    parent->addAsChild(item, prev);
    emit doc->itemAdded(item);
    this->item = item;
    emit doc->itemFocused(item);
}

void CreateAction::revert()
{
    LogItem *newFocusedItem = item->getPrev()?item->getPrev():item->getParent();

    item->detachFromTree();

    emit doc->itemDeleted(item);
    emit doc->itemFocused(newFocusedItem);
}

EditAction::EditAction(LogControl *doc, LogItem *item, QString newText)
    : ClientAction(doc)
    , item(item)
    , textBeforeEdit(item->getText())
    , textAfterEdit(newText)
{

}

void EditAction::make()
{
    item->setText(textAfterEdit);
}

void EditAction::revert()
{
    item->setText(textBeforeEdit);
    emit doc->itemTextChanged(item);
    emit doc->itemFocused(item);
}

MoveAction::MoveAction(LogControl *doc, LogItem *item, LogItem *newParent, LogItem *newPrev)
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

void LogControl::fillGui(LogItem *item)
{
    if (item->getParent() != nullptr)
        emit itemAdded(item);
        LogItem *child = item->getChild();
    while (child) {
        child->control = this;
        fillGui(child);
        child = child->getNext();
    }
}

LogItem *LogControl::findItem(LogItem *parent, uint64_t id)
{
    if (parent->getId() == id)
        return parent;
    LogItem *child = parent->getChild();
    while(child) {
        LogItem *temp = findItem(child, id);
        if (temp)
            return temp;
        child = child->getNext();
    }
    return nullptr;
}

void LogControl::doAction(ClientAction *action)
{
    action->make();
    actionList.push_back(action);
    redoActionList.clear();
}

void LogControl::setStatus(DocumentStatus status)
{
    if (status != docStatus)
        docStatus = status;
}

void LogControl::onLoadingDone()
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

LogControl::LogControl(DB* db, QString name)
    : rootItem(new LogItem(this, nullptr))
    , db(db)
    , name(name)
    , docType(DT_CACHED)
    , docStatus(DS_CLOSED)
{
    rootItem->setId(0);
    //rootItem = std::unique_ptr<LogItem>(new LogItem(this));
    connect(db, SIGNAL(loadingDone()), this, SLOT(onLoadingDone()));
}

QString LogControl::getName()
{
    return name;
}

void LogControl::loadData()
{
    docStatus = DS_LOADING;
    db->loadTree(this, rootItem);
}

void LogControl::setRootItem(LogItem *root)
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

LogItem *LogControl::findItemById(uint64_t id)
{
    return findItem(rootItem, id);
}

LogItem *LogControl::getNextItemInTree(LogItem *item)
{
    if (item->getChild() && !item->isChildrenHided())
        return item->getChild();
    else if (item->getNext())
        return item->getNext();
    else {
        LogItem *temp = item->getParent();
        while (temp) {
            if (temp->getNext()) {
                return temp->getNext();
            }
            temp = temp->getParent();
        }
    }
    return item;
}

LogItem *LogControl::getPrevItemInTree(LogItem *item)
{
    if (item == rootItem)
        return rootItem;
    if (item->prev) {
        LogItem *temp = item->prev->getLastChild();
        if (temp && !item->prev->isChildrenHided()) {
            while (temp->getChild())
                temp = temp->getLastChild();
            return temp;
        }
        else {
            return item->getPrev();
        }
    }
    else {
        LogItem *parent = item->getParent();
        return (parent != rootItem)?parent:item;
    }
}

LogItem *LogControl::getRootItem()
{
    return rootItem;
}

void LogControl::addItem(LogItem *item, LogItem *parent, LogItem *prev)
{
    parent->addAsChild(item, prev);
    emit itemAdded(item);
}

void LogControl::removeItem(LogItem *item)
{
    if (item->getChild()) {
        return;
    }
    DeleteAction *action = new DeleteAction(this, item);
    doAction(action);
}

void LogControl::switchFocusTo(LogItem *item, int to)
{
    static const int PAGESTEP = 5;
    LogItem *nextItem = item;
    switch (to) {
    case ME_UP:
        nextItem = getPrevItemInTree(item);
        break;
    case ME_DOWN:
        nextItem = getNextItemInTree(item);
        break;
    case ME_LEFT:
        if (item->getParent() && item->getParent() != rootItem)
            nextItem = item->getParent();
        break;
    case ME_PAGE_UP:
        for (int i = 0; i < PAGESTEP; i++)
            nextItem = getPrevItemInTree(nextItem);
        break;
    case ME_PAGE_DOWN:
        for (int i = 0; i < PAGESTEP; i++)
            nextItem = getNextItemInTree(nextItem);
        break;
    case ME_TO_BEGIN:
        nextItem = rootItem->getChild();
        break;
    case ME_TO_END:
        nextItem = rootItem->getLastChild();
        while(nextItem->getLastChild())
            nextItem = nextItem->getLastChild();
        break;
    default:
        break;
    }
    emit itemFocused(nextItem);
}

void LogControl::createNewItem(LogItem *parent, LogItem *prev)
{
    CreateAction *action = new CreateAction(this, parent, prev);
    doAction(action);
}

void LogControl::moveItem(LogItem *item, int direction)
{
    LogItem *newParent;
    LogItem *newPrev = nullptr;

    switch(direction) {
    case ME_UP:
        if (!item->getPrev())
            return;
        newPrev = item->getPrev()->getPrev();
        newParent = item->getParent();
        break;
    case ME_DOWN:
        if (!item->getNext())
            return;
        newPrev = item->getNext();
        newParent = item->getParent();
        break;
    case ME_LEFT:
        if(item->getParent() == rootItem)
            return;
        newParent = item->getParent()->getParent();
        newPrev = item->getParent();
        break;
    case ME_RIGHT:
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

void LogControl::save()
{
    db->saveTree(rootItem);
}

void LogControl::printItemTree()
{
    std::function<void (LogItem *, QString)> f = [&f](LogItem *item, QString intents) {
        qDebug()  << intents << item->getId() << " : " << item->getText() << " " << ((item->getParent())?(item->getParent()->getId()):-1);
        LogItem *child = item->getChild();
        while (child) {
            f(child, intents + "    ");
            child = child->getNext();
        }
    };
    f(rootItem, QString(""));
}

void LogControl::setItemDone(LogItem *item, bool state)
{
    if (item->isDone() == state)
        return;

    item->setDone(state);
    emit itemDoneChanged(item);
    if (state == true) {
        LogItem *child = item->getChild();
        while (child) {
            setItemDone(child, state);
            child = child->getNext();
        }
    }
}

void LogControl::setItemText(LogItem *item, QString text)
{
    EditAction *action = new EditAction(this, item, text);
    doAction(action);
}

void LogControl::undoLastAction()
{
    if (actionList.size() == 0)
        return;

    ClientAction *action = actionList.back();
    actionList.pop_back();
    action->revert();
    redoActionList.push_back(action);
}

void LogControl::redoAction()
{
    if (redoActionList.empty())
        return;

    ClientAction *action = redoActionList.back();
    redoActionList.pop_back();
    action->make();
    actionList.push_back(action);
}
