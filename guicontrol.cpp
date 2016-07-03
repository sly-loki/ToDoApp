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

void GuiControl::onDocumentOpen(LogControl *doc)
{

}

void GuiControl::onDocumentClose(LogControl *doc)
{

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
        disconnect(this, SIGNAL(itemDoneChanged(LogItem*,bool)), currentDocument, SLOT(setItemDone(LogItem*,bool)));
    }

    initRootWidget();
    LogItem *root = doc->getRootItem();

    LogItem *item = root->getChild();
    while(item) {
        addChildern(item);
        item = item->getNext();
    }

    connect(doc, SIGNAL(itemAdded(LogItem*)), this, SLOT(addItem(LogItem*)));
    connect(doc, SIGNAL(itemDeleted(LogItem*)), this, SLOT(removeItem(LogItem*)));
    connect(doc, SIGNAL(itemModified(LogItem*)), this, SLOT(updateItemPosition(LogItem*)));
    connect(doc, SIGNAL(itemFocused(LogItem*)), this, SLOT(focusItem(LogItem*)));
    connect(doc, SIGNAL(itemDoneChanged(LogItem*)), this, SLOT(setItemDone(LogItem*)));
    connect(this, SIGNAL(itemDoneChanged(LogItem*,bool)), doc, SLOT(setItemDone(LogItem*,bool)));
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

void GuiControl::addItem(LogItem *item)
{
    static int color = 100;
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

    QBoxLayout *layout = new QVBoxLayout();
    QBoxLayout *hLayout = new QHBoxLayout();

    LogTextEdit * newElement = new LogTextEdit(item);
    newElement->setObjectName("itemTextField");
    connect(newElement, SIGNAL(foldCombinationPressed(bool)), this, SLOT(oneOfItemsFoldChanged(bool)));

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

    guiItemsMap[item] = holderWidget;

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
    else
        focusItem(item);
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
        getTextEdit(layout)->setFocus();
        mainScroll->ensureWidgetVisible(getTextEdit(layout));
    }
}

void GuiControl::setItemDone(LogItem *item)
{
    bool done = item->isDone();
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        setDoneState(layout, item->isDone());
    }
}

ItemWidget::ItemWidget()
{

}

void ItemWidget::setText(QString text)
{

}

QString ItemWidget::getText()
{

}
