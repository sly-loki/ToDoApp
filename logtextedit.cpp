#include "logtextedit.h"
#include <QLayout>
#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QScrollArea>
#include <QDebug>



unsigned int LogTextEdit::fontHeight = 1;

void LogTextEdit::updateHeight()
{
    int newLineCount = document()->size().height();
    if (newLineCount != lineCount) {
        if (newLineCount == 0)
            newLineCount = 1;
        lineCount = newLineCount;
        setFixedHeight(lineCount*fontHeight + 12);
    }
}

void LogTextEdit::resizeEvent(QResizeEvent *e)
{
    updateHeight();
    QPlainTextEdit::resizeEvent(e);
}

LogTextEdit::LogTextEdit(LogItem *item, QWidget *parent)
    : QPlainTextEdit(parent)
    , item(item)
    , lineCount(0)
{
    const QFontMetrics fm = fontMetrics();
    LogTextEdit::fontHeight = fm.height();
    setPlainText(item->getText());
    updateHeight();
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

void LogTextEdit::onTextChanged()
{
    item->setText(toPlainText());
    updateHeight();
}

void LogTextEdit::keyPressEvent(QKeyEvent *e)
{
    static std::map<int, MoveEvent> keyToMove =
                {{Qt::Key_Up, ME_UP}, {Qt::Key_Down, ME_DOWN}, {Qt::Key_Left, ME_LEFT}, {Qt::Key_I, ME_UP}, {Qt::Key_K, ME_DOWN}, {Qt::Key_J, ME_LEFT}};

    if (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
        bool processed = true;
        switch (e->key()) {
        case Qt::Key_Q:
            if (e->modifiers() == Qt::ControlModifier)
                item->addNewChild();
            break;
        case Qt::Key_Tab:
            if (!(e->modifiers() & Qt::ShiftModifier))
                item->shiftRight();
            break;
        case Qt::Key_Backtab:
            item->shiftLeft();
            break;
        case Qt::Key_A:
            if (e->modifiers() == Qt::ControlModifier)
                item->addNewSibling();
            break;
        case Qt::Key_R:
            item->remove();
            break;
        case Qt::Key_S:
            if (e->modifiers() == Qt::ControlModifier) {
                item->save();
            } else {
                processed = false;
            }
            break;
        case Qt::Key_Return:
            if (e->modifiers() == Qt::ControlModifier) {
                item->addNewChild();
            } else if (e->modifiers() == Qt::ShiftModifier) {
                item->addNewSibling();
            }
            break;
        case Qt::Key_Up:
        case Qt::Key_Down:
//        case Qt::Key_Left:
        case Qt::Key_J:
        case Qt::Key_I:
        case Qt::Key_K:
            if ((e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) == (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (e->key() == Qt::Key_Up)
                    item->shiftUp();
                else if (e->key() == Qt::Key_Down)
                    item->shiftDown();
            }
            else if (e->modifiers() & Qt::ControlModifier){
                item->switchTo(keyToMove[e->key()]);
            } else {
                processed = false;
            }
            break;
        default:
            processed = false;
            break;
        }
        if (processed) {
            e->accept();
            return;
        }
    } else {
        switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (document()->lineCount() == 1) {
                item->switchTo(keyToMove[e->key()]);
                e->accept();
                return;
            } else {
                QTextCursor cursor = textCursor();
                int position = cursor.positionInBlock();
                const QTextBlock &block = cursor.block();
                QTextLayout *layout = block.layout();
                int lineNumber = layout->lineForTextPosition(position).lineNumber();
                if (lineNumber == 0 && !block.previous().isValid() && e->key() == Qt::Key_Up) {
                    item->switchTo(keyToMove[e->key()]);
                    e->accept();
                    return;
                } else if (lineNumber == layout->lineCount()-1 && !block.next().isValid() && e->key() == Qt::Key_Down) {
                    item->switchTo(keyToMove[e->key()]);
                    e->accept();
                    return;
                }
            }
            break;
        case Qt::Key_Backspace:
            if (toPlainText() == "" && item->getChild() == nullptr)
                item->remove();
            break;
        }
    }
    item->setModified(true);
    QPlainTextEdit::keyPressEvent(e);
}
