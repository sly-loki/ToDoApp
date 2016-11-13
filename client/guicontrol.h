#ifndef GUICONTROL_H
#define GUICONTROL_H

#include <QLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QPushButton>

class ClientItem;
class LogTextEdit;
class LogDocument;
class ClientDocument;

class ItemWidget;

class GuiControl: public QObject
{
    Q_OBJECT

    QWidget *rootWidget;
    QScrollArea *mainScroll;
    std::map<ClientItem*, ItemWidget*> guiItemsMap;
    ClientDocument *currentDocument;

    static LogTextEdit *getTextEdit(QLayout *itemLayout);
    static QCheckBox *getDoneBox(QLayout *itemLayout);
    static QPushButton *getFoldButton(QLayout *itemLayout);

    void setDoneState(QBoxLayout *itemLayout, bool done);
    void initRootWidget();
    void addChildern(ClientItem *item);
    ItemWidget *createItemWidget(ClientItem *item);

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
    void shiftItemToLevel(ClientItem *item, ClientItem *target);
    void unplagItem(ClientItem *item);

public slots:

    void onDocumentOpen(ClientDocument *doc);
    void onDocumentClose(ClientDocument *doc);
    void setCurrentDocument(ClientDocument *doc);

    void addItem(ClientItem *item);
    void removeItem(ClientItem *item);
    void updateItemPosition(ClientItem *item);
    void focusItem(ClientItem *item);
    void setItemDone(ClientItem *item);
    void setItemText(ClientItem *item);

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
