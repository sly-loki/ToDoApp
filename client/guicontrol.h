#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include "itemwidget.h"

class ClientItem;
class LogTextEdit;
class LogDocument;
class ClientDocument;

class ItemWidget;

class DocumentView: public QObject
{
    Q_OBJECT
public:
    QWidget *rootWidget;
    std::map<ClientItem*, ItemWidget*> guiItemsMap;
    ClientDocument *currentDocument;
};

class ItemDragEvent: public QObject
{
    Q_OBJECT
public:
    QPoint startPoint;
    QLabel *dummy;
    QLabel *placeHolder;
    ItemWidget *item;
    ItemWidget *currentSibling;
    ItemWidget *currentParent;
    void grab(ItemWidget *itemWidget);

    ItemDragEvent(ItemWidget *item)
        : dummy(nullptr)
        , placeHolder(new QLabel())
        , item(item)
        , currentSibling(nullptr)
        , currentParent(nullptr)
    {
        placeHolder->resize(item->geometry().size());
        placeHolder->setMinimumSize(item->geometry().size());
        placeHolder->setStyleSheet("background-color: darkgrey");
    }

    ~ItemDragEvent() {
        delete dummy;
        delete placeHolder;
    }
};

class GuiControl: public QObject
{
    Q_OBJECT

    QWidget *rootWidget;
    QTabWidget *tab;
    std::map<ClientItem*, ItemWidget*> guiItemsMap;
    std::map<ClientDocument*, int> docToTab;
    ClientDocument *currentDocument;
    std::map<ClientDocument*, DocumentView *> docToView;

    static LogTextEdit *getTextEdit(QLayout *itemLayout);
    static QCheckBox *getDoneBox(QLayout *itemLayout);
    static QPushButton *getFoldButton(QLayout *itemLayout);

    void setDoneState(QBoxLayout *itemLayout, bool done);
    int initRootWidget();
    void addChildern(ClientItem *item);
    ItemWidget *createItemWidget(ClientItem *item);

    ItemDragEvent *currentDragEvent;

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

    void onItemStartDragging();
    void onItemDragging(QPoint p);
    void onItemDrop();

public:
    GuiControl(QTabWidget *tab);
    void shiftItemToLevel(ClientItem *item, ClientItem *target);
    void unplagItem(ClientItem *item);

public slots:

    void onDocumentTabClosePressed(int index);
    void onDocumentTabChanged(int index);

    void onDocumentOpen(ClientDocument *doc);
    void onDocumentClose(ClientDocument *doc);
    void setCurrentDocument(ClientDocument *doc);

    void showNextDocument();
    void showPrevDocument();

    void addItem(ClientItem *item);
    void removeItem(ClientItem *item);
    void updateItemPosition(ClientItem *item);
    void focusItem(ClientItem *item);
    void setItemDone(ClientItem *item);
    void setItemText(ClientItem *item);
    void hideItem(ClientItem *item);
    void showItem(ClientItem *item);

signals:
    void itemDoneChanged(ClientItem *item, bool state);
    void itemFoldChanged(ClientItem *item, bool state);
    void newItemRequest(ClientItem *parent, ClientItem *prev);
    void removeItemRequest(ClientItem *item);
    void itemTextChanged(ClientItem *item, QString newText);
    void itemPositionChanged(ClientItem *item, ClientItem *newParent, ClientItem *newPrev);
    void itemMoveRequested(ClientItem *item, int direction);
    void itemFocusChanged(ClientItem *item, int direction);
    void undoPressed();
    void redoPressed();
    void savePressed();

};

#endif // GUICONTROL_H
