#include "core.h"
#include <functional>

#include <QDebug>

uint64_t LogItem::nextId = 1;

LogItem::LogItem(LogControl *control, LogItem *parent, uint64_t id)
    : control(control)
    , id(id)
    , type(IT_TODO)
    , modified(false)
    , syncedWithServer(false)
    , parent(parent)
    , next(nullptr)
    , prev(nullptr)
    , firstChild(nullptr)
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

void LogItem::addNewChild()
{
    control->createNewChild(this);
}

void LogItem::addNewSibling()
{
    control->createNewSibling(this);
}

void LogItem::shiftRight()
{
    control->shiftRight(this);
}

void LogItem::shiftLeft()
{
    control->shiftLeft(this);
}

void LogItem::shiftUp()
{
    control->shiftUp(this);
}

void LogItem::shiftDown()
{
    control->shiftDown(this);
}

void LogItem::switchTo(MoveEvent to)
{
    control->switchTo(this, to);
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

DeleteAction::DeleteAction(LogItem *item)
{

}

void DeleteAction::make()
{

}

void DeleteAction::revert()
{

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

LogControl::LogControl(DB* db, QString name)
    : rootItem(new LogItem(this, nullptr))
    , db(db)
    , name(name)
{
    rootItem->setId(0);
    //rootItem = std::unique_ptr<LogItem>(new LogItem(this));
}

QString LogControl::getName()
{
    return name;
}

void LogControl::loadData()
{
    db->loadTree(this, rootItem);
    if (!rootItem->getChild()) {
        createNewChild(rootItem);
    }
    else {
//        fillGui(rootItem);
    }
    printItemTree();
}

void LogControl::setRootItem(LogItem *root)
{
    delete rootItem;
    rootItem = root;
    if (!rootItem->getChild()) {
        createNewChild(nullptr);
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

LogItem *LogControl::createNewChild(LogItem *parent)
{
    if (!parent)
        parent = rootItem;
    LogItem *item = new LogItem(this, parent);
    parent->addAsChild(item);
    emit itemAdded(item);
    return item;
}

LogItem *LogControl::createNewSibling(LogItem *item)
{
    LogItem *parent = item->getParent();
    LogItem *newItem = new LogItem(this, parent);
    parent->addAsChild(newItem, item);
    emit itemAdded(newItem);
    return newItem;
}

void LogControl::shiftRight(LogItem *item)
{
    if (!item->prev)
        return;

    LogItem *newParent = item->prev;
    item->detachFromTree();
    newParent->addAsLastChild(item);

//    guiControl->unplagItem(item);
//    guiControl->shiftItemToLevel(item, item->parent);
    emit itemModified(item);
}

void LogControl::shiftLeft(LogItem *item)
{
    if (item->getParent() == rootItem)
        return;
    LogItem *oldParent = item->getParent();
    LogItem *newParent = oldParent->getParent();

    item->detachFromTree();
    newParent->addAsChild(item, oldParent);

//    guiControl->unplagItem(item);
//    guiControl->shiftItemToLevel(item, item->parent);
    emit itemModified(item);
}

void LogControl::shiftUp(LogItem *item)
{
    if (!item->prev)
        return;

    LogItem *prev = item->prev;
    LogItem *temp = item->next;

    if (item->next)
        item->next->prev = prev;
    item->next = prev;
    item->prev = prev->prev;

    prev->next = temp;
    if (prev->prev)
        prev->prev->next = item;
    prev->prev = item;

    if (item->parent->firstChild == prev)
        item->parent->firstChild = item;

    emit itemModified(item);
//    guiControl->unplagItem(item);
//    guiControl->shiftItemToLevel(item, item->parent);

}

void LogControl::shiftDown(LogItem *item)
{
    if (item->next) {
        item->next->shiftUp();
        emit itemFocused(item);
//        guiControl->switchFocusTo(item);
    }
}

void LogControl::removeItem(LogItem *item)
{
    if (item->getChild()) {

    }
    LogItem *newFocusedItem = item->getPrev()?item->getPrev():item->getParent();
    item->detachFromTree();
    emit itemDeleted(item);
    emit itemFocused(newFocusedItem);
}

void LogControl::switchTo(LogItem *item, MoveEvent to)
{
    switch (to) {
    case ME_UP:
        emit itemFocused(getPrevItemInTree(item));
        break;
    case ME_DOWN:
        emit itemFocused(getNextItemInTree(item));
        break;
    case ME_LEFT:
        if (item->getParent() && item->getParent() != rootItem)
            emit itemFocused(item->getParent());
        break;
    }
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

void LogControl::cancelLastAction()
{

}

LogDocument::LogDocument()
{

}

