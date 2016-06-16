#include "core.h"

#include <QDebug>

uint64_t LogItem::nextId = 1;

LogItem::LogItem(LogControl *control, LogItem *parent, uint64_t id)
    : control(control)
    , id(id)
    , modified(false)
    , parent(parent)
    , next(nullptr)
    , prev(nullptr)
    , firstChild(nullptr)
    , done(false)
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

LogControl::LogControl(DB* db)
    : rootItem(new LogItem(this, nullptr))
    , db(db)
{
    rootItem->setId(0);
    //rootItem = std::unique_ptr<LogItem>(new LogItem(this));
}

void LogControl::loadData()
{
    db->loadTree(rootItem);
    if (!rootItem->getChild()) {
        createNewChild(nullptr);
    }
    else {
        fillGui(rootItem);
    }
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
    item->detachFromTree();
    emit itemDeleted(item);
    emit itemFocused(item->getParent());
}

void LogControl::switchTo(LogItem *item, MoveEvent to)
{
    LogItem *targetItem = nullptr;

    switch (to) {
    case ME_UP:
        if (item == rootItem)
            return;
        if (item->prev) {
            LogItem *temp = item->prev->getLastChild();
            if (temp) {
                while (temp->getChild())
                    temp = temp->getLastChild();
                emit itemFocused(temp);
            }
            else {
                emit itemFocused(item->prev);
            }
        }
        else {
            emit itemFocused(item->parent);
        }
        break;
    case ME_DOWN:
        if (item->getChild())
            emit itemFocused(item->getChild());
        else if (item->getNext())
            emit itemFocused(item->getNext());
        else {
            LogItem *temp = item->getParent();
            while (temp) {
                if (temp->getNext()) {
                    emit itemFocused(temp->getNext());
                    break;
                }
                temp = temp->getParent();
            }
        }
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
