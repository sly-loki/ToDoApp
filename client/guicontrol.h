#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QPushButton>

class LogItem;
class LogTextEdit;
class LogDocument;
class LogControl;

class ItemWidget;

class GuiControl: public QObject
{
    Q_OBJECT

    QWidget *rootWidget;
    QScrollArea *mainScroll;
    std::map<LogItem*, ItemWidget*> guiItemsMap;
    LogControl *currentDocument;

    static LogTextEdit *getTextEdit(QLayout *itemLayout);
    static QCheckBox *getDoneBox(QLayout *itemLayout);
    static QPushButton *getFoldButton(QLayout *itemLayout);

    void setDoneState(QBoxLayout *itemLayout, bool done);
    void initRootWidget();
    void addChildern(LogItem *item);
    ItemWidget *createItemWidget(LogItem *item);

protected slots:
    void onNewChildPressed();
    void onNewSiblingPressed();
    void onItemTextChanged();
    void onItemFocusChanged(int direction);
    void oneOfItemsDoneChanged(int state);
    void oneOfItemsFoldChanged(bool folded);
    void onItemMovePressed(int direction);
    void onItemRemovePressed();
    void onItemDoneChanged(bool done);

public:
    GuiControl(QScrollArea *scroll);
    void shiftItemToLevel(LogItem *item, LogItem *target);
    void unplagItem(LogItem *item);

public slots:

    void onDocumentOpen(LogControl *doc);
    void onDocumentClose(LogControl *doc);
    void setCurrentDocument(LogControl *doc);

    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void updateItemPosition(LogItem *item);
    void focusItem(LogItem *item);
    void setItemDone(LogItem *item);
    void setItemText(LogItem *item);

signals:
    void itemDoneChanged(LogItem *item, bool state);
    void itemFoldChanged(LogItem *item, bool state);
    void newItemRequest(LogItem *parent, LogItem *prev);
    void removeItemRequest(LogItem *item);
    void itemTextChanged(LogItem *item, QString newText);
    void itemPositionChanged(LogItem *item, LogItem *newParent, LogItem *newPrev);
    void itemMoveRequested(LogItem *item, int direction);
    void itemFocusChanged(LogItem *item, int direction);
    void undoPressed();
    void redoPressed();
    void savePressed();

};

#endif // GUICONTROL_H
