#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QLayout>
#include <memory>
#include <map>
#include <vector>

#include "guicontrol.h"
#include "storage.h"


class LogControl;
class LogItem;




enum MoveEvent {
    ME_UP,
    ME_DOWN,
    ME_LEFT
};

class LogItem
{
    uint64_t id;
    LogControl *control;
    LogItem *parent;
    LogItem *next;
    LogItem *prev;
    LogItem *firstChild;
    QString text;

    QWidget *guiElement;
    bool modified;

    static uint64_t nextId;

    friend class LogControl;
    friend class DB;

public:

    LogItem(LogControl *control, LogItem *parent, uint64_t id = 0);
    void addNewChild();
    void addNewSibling();
    void shiftRight();
    void shiftLeft();
    void shiftUp();
    void shiftDown();
    void switchTo(MoveEvent to);
    void remove();
    void detachFromTree();
    void addAsChild(LogItem *item, LogItem *after = nullptr);
    void addAsLastChild(LogItem *item);

    void setGui(QWidget *widget);
    const QString &getText() {return text;}
    void setText(const QString newText) {text = newText;}
    LogItem *getParent() {return parent;}
    LogItem *getChild() {return firstChild;}
    LogItem *getLastChild() {
        if (!firstChild)
            return nullptr;
        LogItem *temp = firstChild;
        while(temp->next)
            temp = temp->next;
        return temp;
    }
    LogItem *getNext() {return next;}
    LogItem *getPrev() {return prev;}

    void save();

    bool isModified() {return modified;}
    void setModified(bool status) {modified = status;}

    uint64_t getId() const;
    void setId(const uint64_t &value);
};

class LogControl
{
    LogItem *rootItem;
    GuiControl *guiControl;
    DB *db;

    void fillGui(LogItem *item);
public:
    LogControl(GuiControl *gui, DB* db);
    LogItem *createNewChild(LogItem *parent);
    LogItem *createNewSibling(LogItem *item);
    void shiftRight(LogItem *item);
    void shiftLeft(LogItem *item);
    void shiftUp(LogItem *item);
    void shiftDown(LogItem *item);
    void removeItem(LogItem *item);
    void switchTo(LogItem *item, MoveEvent to);
    void save();
};

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    LogItem *item;
    unsigned int lineCount;

    static unsigned int fontHeight;

    void updateHeight();

public:
    LogTextEdit(LogItem *item, QWidget *parent = nullptr);

public slots:
    void onTextChanged();

protected:
    void keyPressEvent(QKeyEvent *e);
};

#endif // LOGTEXTEDIT_H
