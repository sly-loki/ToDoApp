#include "logtextedit.h"
#include <QLayout>
#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QScrollArea>
#include <QDebug>

GuiControl::GuiControl(QWidget *rootWidget)
    : rootWidget(rootWidget)
{
    QLayout *layout = new QVBoxLayout();
    rootWidget->setLayout(layout);
}

void GuiControl::createNewSiblingElement(LogItem *item)
{
//    QBoxLayout *parentLayout = nullptr;
//    if (!item)
//        return;

//    LogItem *parent = item->getParent();
//    auto it = guiItemsMap.find(parent);
//    if (it != guiItemsMap.end())
//        parentLayout = (*it).second;

//    if (!parentLayout)
//        parentLayout = (QBoxLayout*)(rootWidget->layout());;

//    QBoxLayout *layout = new QVBoxLayout();
//    QWidget * newElement = new LogTextEdit(item);
//    layout->addWidget(newElement);
//    layout->setContentsMargins(50,0,0,0);
//    parentLayout->addLayout(layout);
//    guiItemsMap[item] = layout;

//    newElement->setFocus();
}

void GuiControl::createNewChildElement(LogItem *item)
{
    QBoxLayout *parentLayout = nullptr;
    if (!item)
        return;

    LogItem *parent = item->getParent();
    auto it = guiItemsMap.find(parent);
    if (it != guiItemsMap.end())
        parentLayout = (QBoxLayout*)((*it).second->layout());

    if (!parentLayout)
        parentLayout = (QBoxLayout*)(rootWidget->layout());;

    QBoxLayout *layout = new QVBoxLayout();
    QWidget * newElement = new LogTextEdit(item);
    QWidget *holderWidget = new QGroupBox();
    static int count = 0;
    ((QGroupBox *)holderWidget)->setTitle(QString("%1").arg(count));
    count++;
    layout->addWidget(newElement);
    layout->setContentsMargins(50,0,0,0);
    holderWidget->setLayout(layout);

    guiItemsMap[item] = holderWidget;

/////////////
    QWidget *prevWidget = nullptr;
    if (item->getPrev()) {
        auto it = guiItemsMap.find(item->getPrev());
        if (it != guiItemsMap.end()) {
            prevWidget = (*it).second;
        }
    }
    if (prevWidget) {
        int index = parentLayout->indexOf(prevWidget);
        parentLayout->insertWidget(index+1, holderWidget);
    }
    else {
        parentLayout->insertWidget(1, holderWidget);
    }
/////////

    ((QScrollArea*)rootWidget)->adjustSize();
    newElement->setFocus();

}

void GuiControl::switchFocusTo(LogItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        layout->itemAt(0)->widget()->setFocus();
    }
}

void GuiControl::shiftItemToLevel(LogItem *item, LogItem *target)
{
    QWidget *itemWidget = guiItemsMap[item];

    auto it = guiItemsMap.find(target);

    QBoxLayout *targetLayout = nullptr;
    if (it != guiItemsMap.end()) {
        targetLayout = (QBoxLayout*)((*it).second->layout());
    }
    else {
        targetLayout = (QBoxLayout*)(rootWidget->layout());
    }


    QWidget *prevWidget = nullptr;
    if (item->getPrev()) {
        auto it = guiItemsMap.find(item->getPrev());
        if (it != guiItemsMap.end()) {
            prevWidget = (*it).second;
        }
    }
    if (prevWidget) {
        int index = targetLayout->indexOf(prevWidget);
        targetLayout->insertWidget(index+1, itemWidget);
    }
    else {
        if (targetLayout == rootWidget->layout())
            targetLayout->insertWidget(0, itemWidget);
        else
            targetLayout->insertWidget(1, itemWidget);
    }

    itemWidget->layout()->itemAt(0)->widget()->setFocus();
}

void GuiControl::unplagItem(LogItem *item)
{
//    QBoxLayout *itemLayout = guiItemsMap[item];
    auto it = guiItemsMap.find(item->getParent());

    QBoxLayout *parentLayout = nullptr;
    if (it != guiItemsMap.end()) {
        parentLayout = (QBoxLayout*)((*it).second->layout());
    }
    else {
        parentLayout = (QBoxLayout*)(rootWidget->layout());
    }
    parentLayout->removeWidget(guiItemsMap[item]);
    guiItemsMap[item]->setParent(nullptr);
}


DB::DB()
{

}

void DB::saveItem(LogItem *item, const QString &text)
{

}

void DB::saveTree(LogItem *rootItem)
{

}

ItemVector DB::getFirstLevelItems()
{

}

ItemVector DB::getChildsOf(LogItem *item)
{

}

QString DB::getText(LogItem *item)
{

}


void XmlDB::saveNode(QXmlStreamWriter &stream, LogItem *node)
{
    stream.writeStartElement("item");
    stream.writeTextElement("id", QString("%1").arg(node->getId()));
    stream.writeTextElement("parent", QString("%1").arg(node->getParent()->getId()));
    stream.writeTextElement("text", "Qt Project");
    stream.writeEndElement();
    LogItem *child = node->getChild();
    while(child) {
        saveNode(stream, child);
        child = child->getNext();
    }
}

XmlDB::XmlDB(const QString fileName)
    : fileIsOk(false)
{
    dbFile.setFileName(fileName);
    if (dbFile.open(QIODevice::ReadOnly)) {
        xmlStream.setDevice(&dbFile);
        fileIsOk = true;
    }
}

void XmlDB::saveItem(LogItem *item, const QString &text)
{
//    item->getId();
}

void XmlDB::saveTree(LogItem *rootItem)
{
//    if (fileIsOk)
//        return;
    QFile output("/home/andrei/temp/test.xml");
    output.open(QIODevice::WriteOnly);
    xmlWriter.setDevice(&output);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    LogItem *item = rootItem->getChild();
    while(item) {
        saveNode(xmlWriter, item);
        item = item->getNext();
    }
    xmlWriter.writeEndDocument();
    output.close();
}

void XmlDB::loadTree(LogItem *rootItem)
{
    QFile input("/home/andrei/temp/test.xml");
    input.open(QIODevice::ReadOnly);
    xmlStream.setDevice(&input);
    input.close();
}

ItemVector XmlDB::getFirstLevelItems()
{
    ItemVector result;
    return result;
}

ItemVector XmlDB::getChildsOf(LogItem *item)
{
    ItemVector result;
    return result;
}

QString XmlDB::getText(LogItem *item)
{
    return "";
}

XmlDB::~XmlDB()
{

}

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

void LogItem::addAsChild(LogItem *item)
{
    if (firstChild) {
        firstChild->prev = item;
    }
    item->next = firstChild;
    firstChild = item;
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

LogControl::LogControl(GuiControl *gui, DB* db)
    : rootItem(new LogItem(this, nullptr))
    , guiControl(gui)
    , db(db)
{
    rootItem->setId(0);
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
    LogItem *newItem = new LogItem(this, item->getParent());
    item->getParent()->addAsLastChild(newItem);
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
    LogItem *newParent = item->getParent()->getParent();

    item->detachFromTree();
    newParent->addAsChild(item);

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

LogTextEdit::LogTextEdit(LogItem *item, QWidget *parent)
    : QPlainTextEdit(parent)
    , item(item)
{

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
