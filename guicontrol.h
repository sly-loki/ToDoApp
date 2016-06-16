#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>

class LogItem;
class LogTextEdit;

class GuiControl: public QObject
{
    Q_OBJECT

    QWidget *rootWidget;
    QScrollArea *mainScroll;
    std::map<LogItem*, QWidget*> guiItemsMap;
    void setDoneState(LogTextEdit *edit, bool state);

public:
    GuiControl(QScrollArea *scroll);
    void createNewElement(LogItem *item);
    void switchFocusTo(LogItem *item);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);

public slots:
    void oneOfItemsDoneChange(int state);
};

#endif // GUICONTROL_H
