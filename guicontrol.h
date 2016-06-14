#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>

class LogItem;

class GuiControl
{
    QWidget *rootWidget;
    QScrollArea *mainScroll;
    std::map<LogItem*, QWidget*> guiItemsMap;
public:
    GuiControl(QScrollArea *scroll);
    void createNewSiblingElement(LogItem *item);
    void createNewChildElement(LogItem *item);
    void switchFocusTo(LogItem *item);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);
};

#endif // GUICONTROL_H
