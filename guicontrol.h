#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>

class LogItem;
class LogTextEdit;
class LogDocument;
class LogControl;

class GuiControl: public QObject
{
    Q_OBJECT

    QWidget *rootWidget;
    QScrollArea *mainScroll;
    std::map<LogItem*, QWidget*> guiItemsMap;
    LogControl *currentDocument;

    static LogTextEdit *getTextEdit(QLayout *itemLayout);

    void setDoneState(QBoxLayout *itemLayout, bool done);
    void initRootWidget();
    void addChildern(LogItem *item);

public:
    GuiControl(QScrollArea *scroll);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);

signals:
    void itemDoneChanged(LogItem *item, bool state);
    void newItemRequest(LogItem *parent, LogItem *prev);
    void removeItemRequest(LogItem *item);
    void itemTextChanged(LogItem *item);
    void itemPositionChanged(LogItem *item, LogItem *newParent, LogItem *newPrev);

public slots:

    void onDocumentOpen(LogControl *doc);
    void onDocumentClose(LogControl *doc);
    void setCurrentDocument(LogControl *doc);

    void oneOfItemsDoneChanged(int state);

    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void updateItemPosition(LogItem *item);
    void focusItem(LogItem *item);
    void setItemDone(LogItem *item);

};

#endif // GUICONTROL_H
