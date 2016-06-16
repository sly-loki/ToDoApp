#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QLayout>
#include <memory>
#include <map>
#include <vector>

#include "guicontrol.h"
#include "storage.h"
#include "core.h"

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    LogItem *item;
    unsigned int lineCount;

    static unsigned int fontHeight;

    void updateHeight();

public:
    LogTextEdit(LogItem *item, QWidget *parent = nullptr);
    LogItem *getItem() {return item;}

public slots:
    void onTextChanged();

protected:
    void keyPressEvent(QKeyEvent *e);
};

#endif // LOGTEXTEDIT_H
