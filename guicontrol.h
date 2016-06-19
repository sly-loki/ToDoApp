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
    void setDoneState(QBoxLayout *itemLayout, bool done);
    static LogTextEdit *getTextEdit(QLayout *itemLayout);

public:
    GuiControl(QScrollArea *scroll);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);

signals:
    void itemDoneChanged(LogItem *item, bool state);

public slots:
    void oneOfItemsDoneChanged(int state);

    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void updateItem(LogItem *item);
    void focusItem(LogItem *item);
    void setItemDone(LogItem *item);

};

#endif // GUICONTROL_H
