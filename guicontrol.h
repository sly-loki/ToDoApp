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
    std::map<LogItem*, QWidget*> guiItemsMap;
    LogControl *currentDocument;

    static LogTextEdit *getTextEdit(QLayout *itemLayout);
    static QCheckBox *getDoneBox(QLayout *itemLayout);
    static QPushButton *getFoldButton(QLayout *itemLayout);

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
    void itemTextChanged(LogItem *item, QString newText);
    void itemPositionChanged(LogItem *item, LogItem *newParent, LogItem *newPrev);

protected slots:
    void onNewChildPressed();
    void onNewSiblingPressed();
    void onItemTextChanged();
    void oneOfItemsDoneChanged(int state);
    void oneOfItemsFoldChanged(bool folded);

public slots:

    void onDocumentOpen(LogControl *doc);
    void onDocumentClose(LogControl *doc);
    void setCurrentDocument(LogControl *doc);

    void addItem(LogItem *item);
    void removeItem(LogItem *item);
    void updateItemPosition(LogItem *item);
    void focusItem(LogItem *item);
    void setItemDone(LogItem *item);

};

class ItemWidget: public QWidget
{
    Q_OBJECT
public:
    ItemWidget();
    QString getText();

public slots:
    void addChild(ItemWidget *child, ItemWidget *after);
    void setText(QString text);

signals:
    void foldChanged(bool folded);
    void doneChanged(bool done);
    void textChanged(QString text);
    void newSiblingPressed();
    void newChildPressed();
    void removePressed();
    void moved();
    void focusSwitched();
};

#endif // GUICONTROL_H
