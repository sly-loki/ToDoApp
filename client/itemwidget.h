#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLayout>

#include <vector>

class LogTextEdit;
class LogItem;

class ItemWidget: public QWidget
{
    Q_OBJECT
    LogItem *item;
    QCheckBox *doneBox;
    QPushButton *foldWidget;
    LogTextEdit * textField;

    bool folded;

    QBoxLayout *layout;
    QBoxLayout *childrenLayout;
    std::vector<ItemWidget *> children;

public:
    ItemWidget(LogItem *item);
    QString getText();
    bool isFolded() {return folded;}
    bool isDone() {return doneBox->isChecked();}
    LogItem *getLogItem() {return item;}

public slots:
    void addChild(ItemWidget *child, ItemWidget *after);
    void addChild(ItemWidget *child, size_t position);
    void unplagChild(ItemWidget *child);
    void setText(QString text);
    void setFold(bool folded);
    void setDone(bool done);
    void setFocus();

signals:
    void foldChanged(bool folded);
    void doneChanged(bool done);
    void textChanged();
    void newSiblingPressed();
    void newChildPressed();
    void removePressed();
    void movePressed(int);
    void switchFocusPressed(int);
    void savePressed();
    void undoPressed();
    void redoPressed();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

protected slots:
    void onFoldChanged();
    void onDoneClicked(int done);
    void onDonePressed();
    void onTextChanged();
};

#endif // ITEMWIDGET_H