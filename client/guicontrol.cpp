#include "guicontrol.h"
#include "logtextedit.h"
#include "itemwidget.h"

#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>


void GuiControl::setDoneState(QBoxLayout *itemLayout, bool done)
{
    LogTextEdit *edit = getTextEdit(itemLayout);
    QFont font = edit->font();
    font.setStrikeOut(done);

    edit->setFont(font);
    QColor color = done?Qt::lightGray:Qt::black;
    QString style = QString("color:%1").arg(color.name());
    edit->setStyleSheet(style);

//    edit->setDisabled(done);

    QCheckBox *cb = getDoneBox(itemLayout);
    cb->setChecked(done);

}

LogTextEdit *GuiControl::getTextEdit(QLayout *itemLayout)
{
    return (LogTextEdit *)itemLayout->itemAt(0)->layout()->itemAt(2)->widget();
}

QCheckBox *GuiControl::getDoneBox(QLayout *itemLayout)
{
    return (QCheckBox *)itemLayout->itemAt(0)->layout()->itemAt(1)->widget();
}

QPushButton *GuiControl::getFoldButton(QLayout *itemLayout)
{
    return (QPushButton *)itemLayout->itemAt(0)->layout()->itemAt(0)->widget();
}

void GuiControl::initRootWidget()
{
    rootWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
//    layout->setContentsMargins(0,0,0,0);
    QSpacerItem *spacer = new QSpacerItem(1,1000, QSizePolicy::Expanding, QSizePolicy::Expanding);
    //spacer->
    layout->addSpacerItem(spacer);
    this->rootWidget->setLayout(layout);
//    rootWidget->setStyleSheet("background-color:red;");

    mainScroll->setWidget(this->rootWidget);
}

void GuiControl::addChildern(ClientItem *item)
{
    addItem(item);
    ClientItem *child = item->getChild();
    while (child) {
        addChildern(child);
        child = child->getNext();
    }
}

ItemWidget *GuiControl::createItemWidget(ClientItem *item)
{
    ItemWidget *itemWidget = new ItemWidget(item);

    connect(itemWidget, SIGNAL(foldChanged(bool)), this, SLOT(oneOfItemsFoldChanged(bool)));
    connect(itemWidget, SIGNAL(newSiblingPressed()), this, SLOT(onNewSiblingPressed()));
    connect(itemWidget, SIGNAL(newChildPressed()), this, SLOT(onNewChildPressed()));
    connect(itemWidget, SIGNAL(textChanged()), this, SLOT(onItemTextChanged()));
    connect(itemWidget, SIGNAL(switchFocusPressed(int)), this, SLOT(onItemFocusChanged(int)));
    connect(itemWidget, SIGNAL(movePressed(int)), this, SLOT(onItemMovePressed(int)));
    connect(itemWidget, SIGNAL(undoPressed()), this, SIGNAL(undoPressed()));
    connect(itemWidget, SIGNAL(redoPressed()), this, SIGNAL(redoPressed()));
    connect(itemWidget, SIGNAL(removePressed()), this, SLOT(onItemRemovePressed()));
    connect(itemWidget, SIGNAL(savePressed()), this, SIGNAL(savePressed()));
    connect(itemWidget, SIGNAL(doneChanged(bool)), this, SLOT(onItemDoneChanged(bool)));

    return itemWidget;

}

GuiControl::GuiControl(QScrollArea *scroll)
    : rootWidget(nullptr)
    , mainScroll(scroll)
    , currentDocument(nullptr)
{
//    initRootWidget();
}

void GuiControl::shiftItemToLevel(ClientItem *item, ClientItem *target)
{
    ItemWidget *itemWidget = guiItemsMap[item];

    auto it = guiItemsMap.find(target);
    ItemWidget *prevWidget = nullptr;
    if (item->getPrev())
        prevWidget = guiItemsMap[item->getPrev()];

    QBoxLayout *targetLayout = nullptr;
    if (it != guiItemsMap.end()) {
        ItemWidget *parentItem = (*it).second;
        parentItem->addChild(itemWidget, prevWidget);
    }
    else {
        targetLayout = (QBoxLayout*)(rootWidget->layout());
        if (prevWidget) {
            int index = targetLayout->indexOf(prevWidget) + 1;
            targetLayout->insertWidget(index, itemWidget);
        }
        else {
            targetLayout->insertWidget(0, itemWidget);
        }
    }

    itemWidget->setFocus();
}

void GuiControl::unplagItem(ClientItem *item)
{
    auto it = guiItemsMap.find(item->getParent());
    ItemWidget *itemWidget = guiItemsMap[item];

    if (it != guiItemsMap.end()) {
        ItemWidget *parentItem = (*it).second;
        parentItem->unplagChild(itemWidget);
    }
    else {
        QBoxLayout *parentLayout = (QBoxLayout*)(rootWidget->layout());
        parentLayout->removeWidget(itemWidget);
        itemWidget->setParent(nullptr);
    }
}

void GuiControl::onNewChildPressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    emit newItemRequest(textEdit->getItem(), nullptr);
}

void GuiControl::onNewSiblingPressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    ClientItem *item = textEdit->getItem();
    emit newItemRequest(item->getParent(), item);
}

void GuiControl::onItemTextChanged()
{

    ItemWidget *itemWidget = (ItemWidget *)sender();
    ClientItem *item = itemWidget->getLogItem();
    QString text = itemWidget->getText();

    emit itemTextChanged(item, text);
}

void GuiControl::onItemFocusChanged(int direction)
{
    ItemWidget *itemWidget = (ItemWidget *)sender();
    ClientItem *item = itemWidget->getLogItem();
    emit itemFocusChanged(item, direction);
}

void GuiControl::onDocumentOpen(ClientDocument *doc)
{
    Q_UNUSED(doc);
}

void GuiControl::onDocumentClose(ClientDocument *doc)
{
    Q_UNUSED(doc);
}

void GuiControl::setCurrentDocument(ClientDocument *doc)
{
    if (doc == currentDocument)
        return;

    if (currentDocument) {
        disconnect(currentDocument, SIGNAL(itemAdded(ClientItem*)), this, SLOT(addItem(ClientItem*)));
        disconnect(currentDocument, SIGNAL(itemDeleted(ClientItem*)), this, SLOT(removeItem(ClientItem*)));
        disconnect(currentDocument, SIGNAL(itemModified(ClientItem*)), this, SLOT(updateItemPosition(ClientItem*)));
        disconnect(currentDocument, SIGNAL(itemFocused(ClientItem*)), this, SLOT(focusItem(ClientItem*)));
        disconnect(currentDocument, SIGNAL(itemDoneChanged(ClientItem*)), this, SLOT(setItemDone(ClientItem*)));
        disconnect(currentDocument, SIGNAL(itemTextChanged(ClientItem*)), this, SLOT(setItemText(ClientItem*)));
        disconnect(this, SIGNAL(itemDoneChanged(ClientItem*,bool)), currentDocument, SLOT(setItemDone(ClientItem*,bool)));
        disconnect(this, SIGNAL(itemFoldChanged(ClientItem*,bool)), currentDocument, SLOT(setItemFold(ClientItem*,bool)));
        disconnect(this, SIGNAL(itemFocusChanged(ClientItem*,int)), currentDocument, SLOT(switchFocusTo(ClientItem*,int)));
        disconnect(this, SIGNAL(newItemRequest(ClientItem*,ClientItem*)), currentDocument, SLOT(createNewItem(ClientItem*,ClientItem*)));
        disconnect(this, SIGNAL(itemMoveRequested(ClientItem*,int)), currentDocument, SLOT(moveItem(ClientItem*,int)));
        disconnect(this, SIGNAL(undoPressed()), currentDocument, SLOT(undoLastAction()));
        disconnect(this, SIGNAL(redoPressed()), currentDocument, SLOT(redoAction()));
        disconnect(this, SIGNAL(removeItemRequest(ClientItem*)), currentDocument, SLOT(removeItem(ClientItem*)));
        disconnect(this, SIGNAL(savePressed()), currentDocument, SLOT(save()));
        disconnect(this, SIGNAL(itemTextChanged(ClientItem*,QString)), currentDocument, SLOT(setItemText(ClientItem*,QString)));
    }

    initRootWidget();
//    if (doc->getStatus() != DS_OPEN)
//        return;

    ClientItem *root = doc->getRootItem();

    ClientItem *item = root->getChild();
    while(item) {
        addChildern(item);
        item = item->getNext();
    }

    connect(doc, SIGNAL(itemAdded(ClientItem*)), this, SLOT(addItem(ClientItem*)));
    connect(doc, SIGNAL(itemDeleted(ClientItem*)), this, SLOT(removeItem(ClientItem*)));
    connect(doc, SIGNAL(itemModified(ClientItem*)), this, SLOT(updateItemPosition(ClientItem*)));
    connect(doc, SIGNAL(itemFocused(ClientItem*)), this, SLOT(focusItem(ClientItem*)), Qt::QueuedConnection);
    connect(doc, SIGNAL(itemDoneChanged(ClientItem*)), this, SLOT(setItemDone(ClientItem*)));
//    connect(doc, SIGNAL(itemFoldChanged(LogItem*)), this, SLOT(set)
    connect(doc, SIGNAL(itemTextChanged(ClientItem*)), this, SLOT(setItemText(ClientItem*)));
    connect(this, SIGNAL(itemDoneChanged(ClientItem*,bool)), doc, SLOT(setItemDone(ClientItem*,bool)));
    connect(this, SIGNAL(itemFoldChanged(ClientItem*,bool)), doc, SLOT(setItemFold(ClientItem*,bool)));
    connect(this, SIGNAL(itemFocusChanged(ClientItem*,int)), doc, SLOT(switchFocusTo(ClientItem*,int)));
    connect(this, SIGNAL(newItemRequest(ClientItem*,ClientItem*)), doc, SLOT(createNewItem(ClientItem*,ClientItem*)));
    connect(this, SIGNAL(itemMoveRequested(ClientItem*,int)), doc, SLOT(moveItem(ClientItem*,int)));
    connect(this, SIGNAL(undoPressed()), doc, SLOT(undoLastAction()));
    connect(this, SIGNAL(redoPressed()), doc, SLOT(redoAction()));
    connect(this, SIGNAL(removeItemRequest(ClientItem*)), doc, SLOT(removeItem(ClientItem*)));
    connect(this, SIGNAL(savePressed()), doc, SLOT(save()));
    connect(this, SIGNAL(itemTextChanged(ClientItem*,QString)), doc, SLOT(setItemText(ClientItem*,QString)));
    currentDocument = doc;
}

void GuiControl::oneOfItemsDoneChanged(int state)
{
    QWidget *parent = (QWidget*)(sender()->parent());

    LogTextEdit *edit = (LogTextEdit*)parent->findChild<LogTextEdit*>("itemTextField");
    if (edit) {
        ClientItem *item = edit->getItem();
        emit itemDoneChanged(item, state);

    }
}

void GuiControl::oneOfItemsFoldChanged(bool folded)
{
    ItemWidget *itemWidget = (ItemWidget*)(sender());
    itemWidget->setFold(folded);
    ClientItem *item = itemWidget->getLogItem();
    emit itemFoldChanged(item, folded);
}

void GuiControl::onItemMovePressed(int direction)
{
    ItemWidget *itemWidget = (ItemWidget *)sender();
    ClientItem *item = itemWidget->getLogItem();
    emit itemMoveRequested(item, direction);
}

void GuiControl::onItemRemovePressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    ClientItem *item = textEdit->getItem();
    emit removeItemRequest(item);
}

void GuiControl::onItemDoneChanged(bool done)
{
    ItemWidget *itemWidget = (ItemWidget *)sender();
    ClientItem *item = itemWidget->getLogItem();
    emit itemDoneChanged(item, done);
}

void GuiControl::addItem(ClientItem *item)
{
    ItemWidget *parentItem = nullptr;
    if (!item)
        return;

    ClientItem *parent = item->getParent();

    auto it = guiItemsMap.find(parent);
    if (it != guiItemsMap.end())
        parentItem = (ItemWidget*)((*it).second);

    ItemWidget *itemWidget = nullptr;//guiItemsMap[item];
    if (itemWidget == nullptr) {
        itemWidget = createItemWidget(item);
        guiItemsMap[item] = itemWidget;
    }

/////////////
    ItemWidget *prevWidget = nullptr;
    if (item->getPrev()) {
        auto it = guiItemsMap.find(item->getPrev());
        if (it != guiItemsMap.end()) {
            prevWidget = (*it).second;
        }
    }

    if (parentItem)
    {
        parentItem->addChild(itemWidget, prevWidget);
    } else {
        int index = 0;
        QBoxLayout *parentLayout = (QBoxLayout *)rootWidget->layout();
        if (prevWidget)
            index = parentLayout->indexOf(prevWidget) + 1;
        parentLayout->insertWidget(index, itemWidget);
    }

    if (item->isDone())
        itemWidget->setDone(true);
    if (item->isFolded())
        itemWidget->setFold(true);
}

void GuiControl::removeItem(ClientItem *item)
{
    unplagItem(item);
}

void GuiControl::updateItemPosition(ClientItem *item)
{
    unplagItem(item);
    shiftItemToLevel(item, item->getParent());
}

void GuiControl::focusItem(ClientItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        ItemWidget *itemWidget = (*it).second;
        itemWidget->setFocus();
        mainScroll->ensureWidgetVisible(itemWidget);
    }
}

void GuiControl::setItemDone(ClientItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        (*it).second->setDone(item->isDone());
    }
}

void GuiControl::setItemText(ClientItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        (*it).second->setText(item->getText());
    }
}
