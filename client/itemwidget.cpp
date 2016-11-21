#include "itemwidget.h"

#include "logtextedit.h"
#include <assert.h>
#include <algorithm>


ItemWidget::ItemWidget(ClientItem *item)
    : item(item)
    , doneBox(new QCheckBox)
    , textField(new LogTextEdit(item))
    , folded(false)
    , layout(new QVBoxLayout())
{
    textLayout = new QHBoxLayout();

    textField->setObjectName("itemTextField");

    connect(textField, SIGNAL(foldCombinationPressed(bool)), this, SLOT(onFoldChanged()));
    connect(textField, SIGNAL(newSiblingPressed()), this, SIGNAL(newSiblingPressed()));
    connect(textField, SIGNAL(newChildPressed()), this, SIGNAL(newChildPressed()));
    connect(textField, SIGNAL(switchFocusPressed(int)), this, SIGNAL(switchFocusPressed(int)));
    connect(textField, SIGNAL(savePressed()), this, SIGNAL(savePressed()));
    connect(textField, SIGNAL(donePressed()), this, SLOT(onDonePressed()));
    connect(textField, SIGNAL(itemTextChanged()), this, SLOT(onTextChanged()));
    connect(textField, SIGNAL(movePressed(int)), this, SIGNAL(movePressed(int)));
    connect(textField, SIGNAL(undoPressed()), this, SIGNAL(undoPressed()));
    connect(textField, SIGNAL(redoPressed()), this, SIGNAL(redoPressed()));

    connect(textField, SIGNAL(removePressed()), this, SIGNAL(removePressed()));

    foldWidget = new QPushButton();
    foldWidget->setFixedSize(15,15);
    foldWidget->setStyleSheet("background:red");
    foldWidget->setCheckable(true);
    foldWidget->setChecked(item->isFolded());
    textLayout->addWidget(foldWidget);

    connect(foldWidget, SIGNAL(clicked(bool)), this, SLOT(onFoldChanged()));

    if (!item->getChild()) {
        foldWidget->setEnabled(false);
        foldWidget->setStyleSheet("background: grey");
    }

    connect(doneBox, SIGNAL(stateChanged(int)), this, SLOT(onDoneClicked(int)));

    textLayout->setContentsMargins(0,0,0,0);
    if (item->getType() == ItemType::TODO) {
        textLayout->addWidget(doneBox);
    }
    textLayout->addWidget(textField);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    layout->addLayout(textLayout);
    layout->setContentsMargins(50,0,0,0);
    setLayout(layout);

}

void ItemWidget::setText(QString text)
{
    if (text != textField->toPlainText()) {
        textField->blockSignals(true);
        textField->setPlainText(text);
        textField->blockSignals(false);
    }
}

void ItemWidget::setFold(bool folded)
{
    if (folded == this->folded)
        return;

    for (ItemWidget *child : children) {
        child->setVisible(!folded);
    }

    this->folded = folded;
    foldWidget->setChecked(folded);
}

void ItemWidget::setDone(bool done)
{
    QFont font = textField->font();
    font.setStrikeOut(done);

    textField->setFont(font);
    QColor color = done?Qt::lightGray:Qt::black;
    QString style = QString("color:%1").arg(color.name());
    textField->setStyleSheet(style);

    doneBox->setChecked(done);
}

void ItemWidget::setFocus()
{
    textField->setFocus();
}

void ItemWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void ItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void ItemWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void ItemWidget::onFoldChanged()
{
    emit foldChanged(!this->folded);
}

void ItemWidget::onDoneClicked(int done)
{
    emit doneChanged(done);
}

void ItemWidget::onDonePressed()
{
    emit doneChanged(!item->isDone());
}

void ItemWidget::onTextChanged()
{
    emit textChanged();
}

QString ItemWidget::getText()
{
    return textField->toPlainText();
}

void ItemWidget::hide()
{
    textField->setVisible(false);
    doneBox->setVisible(false);
    foldWidget->setVisible(false);
}

void ItemWidget::show()
{
    textField->setVisible(true);
    doneBox->setVisible(true);
    foldWidget->setVisible(true);
}

void ItemWidget::addChild(ItemWidget *child, ItemWidget *after)
{
//    assert(after->parent() == this);

    int index = 0;
    if (after) {
        index = layout->indexOf(after) + 1;
    }
    else {
        index = 1;
    }

    layout->insertWidget(index, child);

    if (isFolded())
        child->setVisible(false);

    if (children.size() == 0) {
        foldWidget->setEnabled(true);
        foldWidget->setStyleSheet("background:red");
    }

    children.push_back(child);
}

void ItemWidget::addChild(ItemWidget *child, size_t position)
{

}

void ItemWidget::unplagChild(ItemWidget *child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it == children.end())
        return;

    layout->removeWidget(child);
    child->setParent(nullptr);
    children.erase(it);
}
