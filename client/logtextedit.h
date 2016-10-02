#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QLayout>
#include <QTimer>
#include <memory>
#include <map>
#include <vector>

#include "guicontrol.h"
#include "storage.h"
#include "logappserver.h"
#include "core.h"

class LogTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    LogItem *item;
    unsigned int lineCount;

    static unsigned int fontHeight;

    QTimer timer;
    uint editCountSinceLastSignal;
    static const int EDIT_TRASHOLD = 10;
    static const int TIME_TRASHOLD_MS = 1000;

    void updateHeight();

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);

public:
    LogTextEdit(LogItem *item, QWidget *parent = nullptr);
    LogItem *getItem() {return item;}

protected slots:
    void onTextChanged();

signals:
    void foldCombinationPressed(bool);
    void undoPressed();
    void redoPressed();
    void newSiblingPressed();
    void newChildPressed();
    void removePressed();
    void movePressed(int);
    void savePressed();
    void switchFocusPressed(int);
    void donePressed();
    void itemTextChanged();
};

#endif // LOGTEXTEDIT_H
