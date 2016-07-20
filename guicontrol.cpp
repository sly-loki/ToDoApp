#include "guicontrol.h"
#include "logtextedit.h"

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

void GuiControl::addChildern(LogItem *item)
{
    addItem(item);
    LogItem *child = item->getChild();
    while (child) {
        addChildern(child);
        child = child->getNext();
    }
}

QWidget *GuiControl::createItemWidget(LogItem *item)
{
    QBoxLayout *layout = new QVBoxLayout();
    QBoxLayout *hLayout = new QHBoxLayout();

    LogTextEdit * newElement = new LogTextEdit(item);
    newElement->setObjectName("itemTextField");
    connect(newElement, SIGNAL(foldCombinationPressed(bool)), this, SLOT(oneOfItemsFoldChanged(bool)));
    connect(newElement, SIGNAL(newSiblingPressed()), this, SLOT(onNewSiblingPressed()));
    connect(newElement, SIGNAL(newChildPressed()), this, SLOT(onNewChildPressed()));
    connect(newElement, SIGNAL(textChanged()), this, SLOT(onItemTextChanged()));
    connect(newElement, SIGNAL(switchFocusPressed(int)), this, SLOT(onItemFocusChanged(int)));
    connect(newElement, SIGNAL(movePressed(int)), this, SLOT(onItemMovePressed(int)));
    connect(newElement, SIGNAL(undoPressed()), this, SIGNAL(undoPressed()));
    connect(newElement, SIGNAL(redoPressed()), this, SIGNAL(redoPressed()));
    connect(newElement, SIGNAL(removePressed()), this, SLOT(onItemRemovePressed()));
    connect(newElement, SIGNAL(savePressed()), this, SIGNAL(savePressed()));
    connect(newElement, SIGNAL(donePressed()), this, SLOT(onItemDonePressed()));

    QPushButton *foldWidget = new QPushButton();
    foldWidget->setFixedSize(15,15);
    foldWidget->setStyleSheet("background:red");
    foldWidget->setCheckable(true);
    foldWidget->setChecked(item->isChildrenHided());
    hLayout->addWidget(foldWidget);
    connect(foldWidget, SIGNAL(clicked(bool)), this, SLOT(oneOfItemsFoldChanged(bool)));
    if (!item->getChild()) {
        foldWidget->setEnabled(false);
        foldWidget->setStyleSheet("background: grey");
    }

    QCheckBox *box = new QCheckBox;
    connect(box, SIGNAL(stateChanged(int)), this, SLOT(oneOfItemsDoneChanged(int)));

    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(box);
    hLayout->addWidget(newElement);

    QWidget *holderWidget = new QWidget();
//    QColor qcolor = QColor(color, color, color);
//    QString style = QString("background-color:%1%2;").arg("#").arg(qcolor.value());
//    color+=10;
//    holderWidget->setStyleSheet(style);
    holderWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    static int count = 0;
//    ((QGroupBox *)holderWidget)->setTitle(QString("%1").arg(count));
    count++;
//    layout->addWidget(newElement);
    layout->addLayout(hLayout);
    layout->setContentsMargins(50,0,0,0);
    holderWidget->setLayout(layout);

    return holderWidget;

}

GuiControl::GuiControl(QScrollArea *scroll)
    : rootWidget(nullptr)
    , mainScroll(scroll)
    , currentDocument(nullptr)
{
//    initRootWidget();
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

//    itemWidget->layout()->itemAt(0)->widget()->setFocus();
    focusItem(item);
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

void GuiControl::onNewChildPressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    emit newItemRequest(textEdit->getItem(), nullptr);
}

void GuiControl::onNewSiblingPressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    LogItem *item = textEdit->getItem();
    emit newItemRequest(item->getParent(), item);
}

void GuiControl::onItemTextChanged()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    QString text = textEdit->toPlainText();
    LogItem *item = textEdit->getItem();
    emit itemTextChanged(item, text);
}

void GuiControl::onItemFocusChanged(int direction)
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    LogItem *item = textEdit->getItem();
    emit itemFocusChanged(item, direction);
}

void GuiControl::onDocumentOpen(LogControl *doc)
{
    Q_UNUSED(doc);
}

void GuiControl::onDocumentClose(LogControl *doc)
{
    Q_UNUSED(doc);
}

void GuiControl::setCurrentDocument(LogControl *doc)
{
    if (doc == currentDocument)
        return;

    if (currentDocument) {
        disconnect(currentDocument, SIGNAL(itemAdded(LogItem*)), this, SLOT(addItem(LogItem*)));
        disconnect(currentDocument, SIGNAL(itemDeleted(LogItem*)), this, SLOT(removeItem(LogItem*)));
        disconnect(currentDocument, SIGNAL(itemModified(LogItem*)), this, SLOT(updateItemPosition(LogItem*)));
        disconnect(currentDocument, SIGNAL(itemFocused(LogItem*)), this, SLOT(focusItem(LogItem*)));
        disconnect(currentDocument, SIGNAL(itemDoneChanged(LogItem*)), this, SLOT(setItemDone(LogItem*)));
        disconnect(currentDocument, SIGNAL(itemTextChanged(LogItem*)), this, SLOT(setItemText(LogItem*)));
        disconnect(this, SIGNAL(itemDoneChanged(LogItem*,bool)), currentDocument, SLOT(setItemDone(LogItem*,bool)));
        disconnect(this, SIGNAL(itemFocusChanged(LogItem*,int)), currentDocument, SLOT(switchFocusTo(LogItem*,int)));
        disconnect(this, SIGNAL(newItemRequest(LogItem*,LogItem*)), currentDocument, SLOT(createNewItem(LogItem*,LogItem*)));
        disconnect(this, SIGNAL(itemMoveRequested(LogItem*,int)), currentDocument, SLOT(moveItem(LogItem*,int)));
        disconnect(this, SIGNAL(undoPressed()), currentDocument, SLOT(undoLastAction()));
        disconnect(this, SIGNAL(redoPressed()), currentDocument, SLOT(redoAction()));
        disconnect(this, SIGNAL(removeItemRequest(LogItem*)), currentDocument, SLOT(removeItem(LogItem*)));
        disconnect(this, SIGNAL(savePressed()), currentDocument, SLOT(save()));
        disconnect(this, SIGNAL(itemTextChanged(LogItem*,QString)), currentDocument, SLOT(setItemText(LogItem*,QString)));
    }

    initRootWidget();
    if (doc->getStatus() != DS_OPEN)
        return;

    LogItem *root = doc->getRootItem();

    LogItem *item = root->getChild();
    while(item) {
        addChildern(item);
        item = item->getNext();
    }

    connect(doc, SIGNAL(itemAdded(LogItem*)), this, SLOT(addItem(LogItem*)));
    connect(doc, SIGNAL(itemDeleted(LogItem*)), this, SLOT(removeItem(LogItem*)));
    connect(doc, SIGNAL(itemModified(LogItem*)), this, SLOT(updateItemPosition(LogItem*)));
    connect(doc, SIGNAL(itemFocused(LogItem*)), this, SLOT(focusItem(LogItem*)), Qt::QueuedConnection);
    connect(doc, SIGNAL(itemDoneChanged(LogItem*)), this, SLOT(setItemDone(LogItem*)));
    connect(doc, SIGNAL(itemTextChanged(LogItem*)), this, SLOT(setItemText(LogItem*)));
    connect(this, SIGNAL(itemDoneChanged(LogItem*,bool)), doc, SLOT(setItemDone(LogItem*,bool)));
    connect(this, SIGNAL(itemFocusChanged(LogItem*,int)), doc, SLOT(switchFocusTo(LogItem*,int)));
    connect(this, SIGNAL(newItemRequest(LogItem*,LogItem*)), doc, SLOT(createNewItem(LogItem*,LogItem*)));
    connect(this, SIGNAL(itemMoveRequested(LogItem*,int)), doc, SLOT(moveItem(LogItem*,int)));
    connect(this, SIGNAL(undoPressed()), doc, SLOT(undoLastAction()));
    connect(this, SIGNAL(redoPressed()), doc, SLOT(redoAction()));
    connect(this, SIGNAL(removeItemRequest(LogItem*)), doc, SLOT(removeItem(LogItem*)));
    connect(this, SIGNAL(savePressed()), doc, SLOT(save()));
    connect(this, SIGNAL(itemTextChanged(LogItem*,QString)), doc, SLOT(setItemText(LogItem*,QString)));
    currentDocument = doc;
}

void GuiControl::oneOfItemsDoneChanged(int state)
{
    QWidget *parent = (QWidget*)(sender()->parent());

    LogTextEdit *edit = (LogTextEdit*)parent->findChild<LogTextEdit*>("itemTextField");
    if (edit) {
        LogItem *item = edit->getItem();
        emit itemDoneChanged(item, state);

    }
}

void GuiControl::oneOfItemsFoldChanged(bool folded)
{
    QWidget *parent = (QWidget*)(sender()->parent());

    LogTextEdit *edit = (LogTextEdit*)parent->findChild<LogTextEdit*>("itemTextField");
    if (edit) {
        LogItem *item = edit->getItem();
        item->setChildrenHided(folded);
        LogItem *child = item->getChild();
        getFoldButton(guiItemsMap[item]->layout())->setChecked(folded);
        while(child) {
            QWidget *cWidget = guiItemsMap[child];
            cWidget->setVisible(!folded);
            child = child->getNext();
        }
    }
}

void GuiControl::onItemMovePressed(int direction)
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    LogItem *item = textEdit->getItem();
    emit itemMoveRequested(item, direction);
}

void GuiControl::onItemRemovePressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    LogItem *item = textEdit->getItem();
    emit removeItemRequest(item);
}

void GuiControl::onItemDonePressed()
{
    LogTextEdit *textEdit = (LogTextEdit *)sender();
    LogItem *item = textEdit->getItem();
    emit itemDoneChanged(item, !item->isDone());
}

void GuiControl::addItem(LogItem *item)
{
    QBoxLayout *parentLayout = nullptr;
    if (!item)
        return;

    LogItem *parent = item->getParent();
    auto it = guiItemsMap.find(parent);
    if (it != guiItemsMap.end())
        parentLayout = (QBoxLayout*)((*it).second->layout());

    if (!parentLayout)
        parentLayout = (QBoxLayout*)(rootWidget->layout());

    if (parent->getId() != 0) {
        getFoldButton(parentLayout)->setEnabled(true);
        getFoldButton(parentLayout)->setStyleSheet("background:red");
    }

    QWidget *holderWidget = nullptr;//guiItemsMap[item];
    if (holderWidget == nullptr) {
        holderWidget = createItemWidget(item);
        guiItemsMap[item] = holderWidget;
    }

/////////////
    QWidget *prevWidget = nullptr;
    if (item->getPrev()) {
        auto it = guiItemsMap.find(item->getPrev());
        if (it != guiItemsMap.end()) {
            prevWidget = (*it).second;
        }
    }

    int index = 0;
    if (prevWidget) {
        index = parentLayout->indexOf(prevWidget) + 1;
    }
    else {
        index = (parentLayout == rootWidget->layout())? 0 : 1;
    }
    parentLayout->insertWidget(index, holderWidget);
    if (parent->isChildrenHided())
        holderWidget->setVisible(false);
/////////

    if (item->isDone())
        setItemDone(item);
//    else
//        focusItem(item);
}

void GuiControl::removeItem(LogItem *item)
{
    unplagItem(item);
}

void GuiControl::updateItemPosition(LogItem *item)
{
    unplagItem(item);
    shiftItemToLevel(item, item->getParent());
}

void GuiControl::focusItem(LogItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        LogTextEdit *textEdit = getTextEdit(layout);
        textEdit->setFocus();
        mainScroll->ensureWidgetVisible(textEdit);
    }
}

void GuiControl::setItemDone(LogItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        setDoneState(layout, item->isDone());
    }
}

void GuiControl::setItemText(LogItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        LogTextEdit *edit = getTextEdit(layout);
        //disconnect(edit, SIGNAL(textChanged()), this, SLOT(onItemTextChanged()));
        edit->blockSignals(true);
        edit->setPlainText(item->getText());
        edit->blockSignals(false);
//        connect(edit, SIGNAL(textChanged()), this, SLOT(onItemTextChanged()));
    }
}

ItemWidget::ItemWidget()
{

}

void ItemWidget::setText(QString text)
{
    Q_UNUSED(text);
}

QString ItemWidget::getText()
{
    return "";
}

void ItemWidget::addChild(ItemWidget *child, ItemWidget *after)
{
    Q_UNUSED(child);
    Q_UNUSED(after);
}
