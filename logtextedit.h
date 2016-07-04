#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QLayout>
#include <memory>
#include <map>
#include <vector>

#include "guicontrol.h"
#include "storage.h"
#include "logappserver.h"
#include "core.h"

enum ApplicationState
{
    AS_START = 0,
    AS_RECEIVING_ITEMS,
    AS_NORMAL
};

class ApplicationTask
{
    uint64_t id;
public:
    ApplicationTask(uint64_t id);
    virtual ~ApplicationTask();

    uint64_t getId();
    virtual bool process(void *data) = 0;
};

class ReadServerItems: public ApplicationTask
{
    std::map<uint64_t, LogItem *> items;
    uint remainintCount;
    LogControl *control;

public:
    ReadServerItems(uint64_t id, LogControl *control, uint count);

    virtual bool process(void *data);
    void processChildren(uint64_t parentId, uint64_t *ids, uint count);
    LogItem *getRootItem();
};

class ApplicationControl : public QObject
{
    Q_OBJECT
    LogAppServer *server;
    ApplicationState state;
    std::vector<ApplicationTask *> asyncTasks;
    ReadServerItems *readAllItemsTask;

public:
    ApplicationControl(LogAppServer *server);
    bool createNewDocument(QString name, QString fullFileName);
    void start();

signals:
    void createdNewDocument(LogControl *doc);

public slots:
    void onItemListReceived(uint64_t *ids, uint count);
    void onItemReceived(ServerItemData data);
    void onItemChildrenReceived(uint64_t parentId, uint64_t *ids, uint count);
};

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    LogItem *item;
    unsigned int lineCount;

    static unsigned int fontHeight;

    void updateHeight();

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);

public:
    LogTextEdit(LogItem *item, QWidget *parent = nullptr);
    LogItem *getItem() {return item;}

public slots:
    void onTextChanged();

signals:
    void foldCombinationPressed(bool);
    void CtrlZPressed();
    void newSiblingPressed();
    void newChildPressed();
    void removePressed();
    void movePressed(int);
    void switchFocusPressed(int);
};

#endif // LOGTEXTEDIT_H
