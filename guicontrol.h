#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>

class LogItem;

class GuiControl
{
    QWidget *rootWidget;
    std::map<LogItem*, QWidget*> guiItemsMap;
public:
    GuiControl(QWidget *rootWidget);
    void createNewSiblingElement(LogItem *item);
    void createNewChildElement(LogItem *item);
    void switchFocusTo(LogItem *item);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);
};

#endif // GUICONTROL_H
