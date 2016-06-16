#include "guicontrol.h"
#include "logtextedit.h"

#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QCheckBox>
#include <QDebug>


void GuiControl::setDoneState(LogTextEdit *edit, bool state)
{
    QFont font = edit->font();
    font.setStrikeOut(state);
    edit->setFont(font);
}

GuiControl::GuiControl(QScrollArea *scroll)
    : rootWidget(new QWidget())
    , mainScroll(scroll)
{
    QVBoxLayout *layout = new QVBoxLayout();
//    layout->setContentsMargins(0,0,0,0);
    QSpacerItem *spacer = new QSpacerItem(1,1000, QSizePolicy::Expanding, QSizePolicy::Expanding);
    //spacer->
    layout->addSpacerItem(spacer);
    this->rootWidget->setLayout(layout);
//    rootWidget->setStyleSheet("background-color:red;");

    mainScroll->setWidget(this->rootWidget);
}

void GuiControl::createNewElement(LogItem *item)
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
        parentLayout = (QBoxLayout*)(rootWidget->layout());;

    QBoxLayout *layout = new QVBoxLayout();
    QBoxLayout *hLayout = new QHBoxLayout();

    LogTextEdit * newElement = new LogTextEdit(item);
    newElement->setObjectName("itemTextField");
    setDoneState(newElement, item->isDone());

    QCheckBox *box = new QCheckBox;
    box->setChecked(item->isDone());
    connect(box, SIGNAL(stateChanged(int)), this, SLOT(oneOfItemsDoneChange(int)));
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
    if (prevWidget) {
        int index = parentLayout->indexOf(prevWidget);
        parentLayout->insertWidget(index+1, holderWidget);
    }
    else {
        if (parentLayout == rootWidget->layout())
            parentLayout->insertWidget(0, holderWidget);
        else
            parentLayout->insertWidget(1, holderWidget);
    }
/////////

    newElement->setFocus();

}

void GuiControl::switchFocusTo(LogItem *item)
{
    auto it = guiItemsMap.find(item);
    if (it != guiItemsMap.end()) {
        QBoxLayout *layout = (QBoxLayout*)((*it).second->layout());
        layout->itemAt(0)->layout()->itemAt(1)->widget()->setFocus();
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

void GuiControl::oneOfItemsDoneChange(int state)
{
    QWidget *parent = (QWidget*)(sender()->parent());
//    auto it = guiItemsMap.find(parent);
//    if (it == guiItemsMap.end())
//        return;
    LogTextEdit *edit = (LogTextEdit*)parent->findChild<LogTextEdit*>("itemTextField");
    if (edit) {
        LogItem *item = edit->getItem();
        item->setDone(state);
        QFont font = edit->font();
        font.setStrikeOut(state);
        edit->setFont(font);
    }
}
