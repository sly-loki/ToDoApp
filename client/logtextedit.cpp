#include "logtextedit.h"
#include <QLayout>
#include <QGroupBox>
#include <QTextBlock>
#include <QTextLayout>
#include <QScrollArea>
#include <QDebug>

#include <cassert>

unsigned int LogTextEdit::fontHeight = 1;

void LogTextEdit::updateHeight()
{
    unsigned int newLineCount = document()->size().height();
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
    , editCountSinceLastSignal(0)
{
    const QFontMetrics fm = fontMetrics();
    LogTextEdit::fontHeight = fm.height();
    setPlainText(item->getText());
    updateHeight();
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    timer.setSingleShot(true);

    connect(&timer, SIGNAL(timeout()), this, SIGNAL(itemTextChanged()));
    connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

void LogTextEdit::onTextChanged()
{
//    item->setText(toPlainText());
    updateHeight();
    editCountSinceLastSignal++;
    if (editCountSinceLastSignal >= EDIT_TRASHOLD) {
        editCountSinceLastSignal = 0;
        timer.stop();
        emit itemTextChanged();
    }
    else {
        timer.start(TIME_TRASHOLD_MS);
    }
}

void LogTextEdit::keyPressEvent(QKeyEvent *e)
{
    static std::map<int, MoveEvent> keyToMove =
                {{Qt::Key_Up, MoveEvent::UP}, {Qt::Key_Down, MoveEvent::DOWN}, {Qt::Key_Left, MoveEvent::LEFT},
                 {Qt::Key_I, MoveEvent::UP}, {Qt::Key_K, MoveEvent::DOWN}, {Qt::Key_J, MoveEvent::LEFT}};

    if (e->modifiers() == Qt::ShiftModifier) {
        bool processed = true;
        switch (e->key()) {
        case Qt::Key_Return:
            emit newSiblingPressed();
            break;
        case Qt::Key_Backtab:
            emit movePressed(MoveEvent::LEFT);
            break;
        default:
            processed = false;
            break;
        }
        if (processed) {
            e->accept();
            return;
        }
    }
    if (e->modifiers() == Qt::ControlModifier) {
        bool processed = true;
        switch (e->key()) {
        case Qt::Key_Z:
            emit undoPressed();
            break;
        case Qt::Key_Tab:
            emit movePressed(MoveEvent::RIGHT);
            break;
        case Qt::Key_R:
            emit removePressed();
            break;
        case Qt::Key_S:
            emit savePressed();
            break;
        case Qt::Key_U:
            emit foldCombinationPressed(!item->isFolded());
            break;
        case Qt::Key_D:
            emit donePressed();
            break;
        case Qt::Key_Return:
            emit newChildPressed();
            break;
        case Qt::Key_Home:
            emit switchFocusPressed(MoveEvent::TO_BEGIN);
            break;
        case Qt::Key_End:
            emit switchFocusPressed(MoveEvent::TO_END);
            break;
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_J:
        case Qt::Key_I:
        case Qt::Key_K:
            emit switchFocusPressed(keyToMove[e->key()]);
            break;
        default:
            processed = false;
            break;
        }
        if (processed) {
            e->accept();
            return;
        }
    }

    if ((e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) == (Qt::ShiftModifier | Qt::ControlModifier)) {
        bool processed = true;

        switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_I:
        case Qt::Key_K:
            if (e->key() == Qt::Key_Up || e->key() == Qt::Key_I)
                emit movePressed(MoveEvent::UP);
            else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_K)
                emit movePressed(MoveEvent::DOWN);
            break;
        case Qt::Key_Z:
            emit redoPressed();
            break;
        default:
            processed = false;
            break;
        }

        if (processed) {
            e->accept();
            return;
        }
    }

    if (e->modifiers() == Qt::NoModifier) {
        bool processed = true;

        switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (document()->lineCount() == 1) {
                emit switchFocusPressed(keyToMove[e->key()]);
            } else {
                QTextCursor cursor = textCursor();
                int position = cursor.positionInBlock();
                const QTextBlock &block = cursor.block();
                QTextLayout *layout = block.layout();
                int lineNumber = layout->lineForTextPosition(position).lineNumber();

                if (lineNumber == 0 && !block.previous().isValid() && e->key() == Qt::Key_Up) {
                    emit switchFocusPressed(keyToMove[e->key()]);
                } else if (lineNumber == layout->lineCount()-1 && !block.next().isValid() && e->key() == Qt::Key_Down) {
                    emit switchFocusPressed(keyToMove[e->key()]);
                } else {
                    processed = false;
                }
            }
            break;
        case Qt::Key_Backspace:
            if (toPlainText() == "" && item->getChild() == nullptr) {
                emit removePressed();
            } else {
                processed = false;
            }
            break;
        case Qt::Key_Tab: {
            QTextCursor cursor = textCursor();
            if (cursor.position() == 0) {
                emit movePressed(MoveEvent::RIGHT);
            } else {
                processed = false;
            }
        }
            break;
        case Qt::Key_PageUp:
            emit switchFocusPressed(MoveEvent::PAGE_UP);
            break;
        case Qt::Key_PageDown:
            emit switchFocusPressed(MoveEvent::PAGE_DOWN);
            break;
        default:
            processed = false;
            break;
        }

        if (processed) {
            e->accept();
            return;
        }
    }
    item->setModified(true);
    QPlainTextEdit::keyPressEvent(e);
}

