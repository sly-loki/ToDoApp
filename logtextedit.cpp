#include "logtextedit.h"
#include <QLayout>
#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QScrollArea>
#include <QDebug>

uint64_t LogItem::nextId = 1;

LogItem::LogItem(LogControl *control, LogItem *parent, uint64_t id)
    : control(control)
    , parent(parent)
    , next(nullptr)
    , prev(nullptr)
    , firstChild(nullptr)
    , guiElement(nullptr)
    , modified(false)
    , id(id)
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

void LogItem::setGui(QWidget *widget)
{
    guiElement = widget;
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
        guiControl->createNewChildElement(item);
    LogItem *child = item->getChild();
    while (child) {
        child->control = this;
        fillGui(child);
        child = child->getNext();
    }
}

LogControl::LogControl(GuiControl *gui, DB* db)
    : rootItem(new LogItem(this, nullptr))
    , guiControl(gui)
    , db(db)
{
    rootItem->setId(0);
    db->loadTree(rootItem);
    if (!rootItem->getChild()) {
        createNewChild(nullptr);
    }
    else {
        fillGui(rootItem);
    }
    //rootItem = std::unique_ptr<LogItem>(new LogItem(this));
}

LogItem *LogControl::createNewChild(LogItem *parent)
{
    if (!parent)
        parent = rootItem;
    LogItem *item = new LogItem(this, parent);
    parent->addAsChild(item);
    guiControl->createNewChildElement(item);
    return item;
}

LogItem *LogControl::createNewSibling(LogItem *item)
{
    LogItem *parent = item->getParent();
    LogItem *newItem = new LogItem(this, parent);
    parent->addAsChild(newItem, item);
    guiControl->createNewChildElement(newItem);
    return newItem;
}

void LogControl::shiftRight(LogItem *item)
{
    if (!item->prev)
        return;

    LogItem *newParent = item->prev;
    item->detachFromTree();
    newParent->addAsLastChild(item);

    guiControl->unplagItem(item);
    guiControl->shiftItemToLevel(item, item->parent);
}

void LogControl::shiftLeft(LogItem *item)
{
    if (item->getParent() == rootItem)
        return;
    LogItem *oldParent = item->getParent();
    LogItem *newParent = oldParent->getParent();

    item->detachFromTree();
    newParent->addAsChild(item, oldParent);

    guiControl->unplagItem(item);
    guiControl->shiftItemToLevel(item, item->parent);
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

    guiControl->unplagItem(item);
    guiControl->shiftItemToLevel(item, item->parent);

}

void LogControl::shiftDown(LogItem *item)
{
    if (item->next) {
        item->next->shiftUp();
        guiControl->switchFocusTo(item);
    }
}

void LogControl::removeItem(LogItem *item)
{
    if (item->getChild()) {

    }
    item->detachFromTree();
    guiControl->unplagItem(item);
    guiControl->switchFocusTo(item->getParent());
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
                guiControl->switchFocusTo(temp);
            }
            else {
                guiControl->switchFocusTo(item->prev);
            }
        }
        else {
            guiControl->switchFocusTo(item->parent);
        }

        break;
    case ME_DOWN:
        if (item->getChild())
            guiControl->switchFocusTo(item->getChild());
        else if (item->getNext())
            guiControl->switchFocusTo(item->getNext());
        else {
            LogItem *temp = item->getParent();
            while (temp) {
                if (temp->getNext()) {
                    guiControl->switchFocusTo(temp->getNext());
                    break;
                }
                temp = temp->getParent();
            }
        }
        break;
    case ME_LEFT:
        if (item->getParent() && item->getParent() != rootItem)
            guiControl->switchFocusTo(item->getParent());
        break;
    }
}

void LogControl::save()
{
    db->saveTree(rootItem);
}

unsigned int LogTextEdit::fontHeight = 1;

void LogTextEdit::updateHeight()
{
    int newLineCount = document()->size().height();
    if (newLineCount != lineCount) {
        if (newLineCount == 0)
            newLineCount = 1;
        lineCount = newLineCount;
        setFixedHeight(lineCount*fontHeight + 10);
    }
}

LogTextEdit::LogTextEdit(LogItem *item, QWidget *parent)
    : QPlainTextEdit(parent)
    , item(item)
    , lineCount(0)
{
    const QFontMetrics fm = fontMetrics();
    LogTextEdit::fontHeight = fm.height();
    setPlainText(item->getText());
    updateHeight();
    connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

void LogTextEdit::onTextChanged()
{
    item->setText(toPlainText());
    updateHeight();
}

void LogTextEdit::keyPressEvent(QKeyEvent *e)
{
    static std::map<int, MoveEvent> keyToMove =
                {{Qt::Key_Up, ME_UP}, {Qt::Key_Down, ME_DOWN}, {Qt::Key_Left, ME_LEFT}};

    if (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
        bool processed = true;
        switch (e->key()) {
        case Qt::Key_Q:
            item->addNewChild();
            break;
        case Qt::Key_Tab:
            if (!(e->modifiers() & Qt::ShiftModifier))
                item->shiftRight();
            break;
        case Qt::Key_Backtab:
            item->shiftLeft();
            break;
        case Qt::Key_A:
            item->addNewSibling();
            break;
        case Qt::Key_R:
            item->remove();
            break;
        case Qt::Key_S:
            item->save();
            break;
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
            if ((e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) == (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (e->key() == Qt::Key_Up)
                    item->shiftUp();
                else if (e->key() == Qt::Key_Down)
                    item->shiftDown();
            }
            else if (e->modifiers() & Qt::ControlModifier){
                item->switchTo(keyToMove[e->key()]);
            } else {
                processed = false;
            }
            break;
        default:
            processed = false;
            break;
        }
        if (processed) {
            e->accept();
            return;
        }
    } else {
        switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (document()->lineCount() == 1) {
                item->switchTo(keyToMove[e->key()]);
                e->accept();
                return;
            } else {
                QTextCursor cursor = textCursor();
                int position = cursor.positionInBlock();
                const QTextBlock &block = cursor.block();
                QTextLayout *layout = block.layout();
                int lineNumber = layout->lineForTextPosition(position).lineNumber();
                if (lineNumber == 0 && !block.previous().isValid() && e->key() == Qt::Key_Up) {
                    item->switchTo(keyToMove[e->key()]);
                    e->accept();
                    return;
                } else if (lineNumber == layout->lineCount()-1 && !block.next().isValid() && e->key() == Qt::Key_Down) {
                    item->switchTo(keyToMove[e->key()]);
                    e->accept();
                    return;
                }
            }
            break;
        }
    }
    item->setModified(true);
    QPlainTextEdit::keyPressEvent(e);
}
