#include "guicontrol.h"
#include "logtextedit.h"

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
