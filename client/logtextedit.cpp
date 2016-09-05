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
                {{Qt::Key_Up, ME_UP}, {Qt::Key_Down, ME_DOWN}, {Qt::Key_Left, ME_LEFT}, {Qt::Key_I, ME_UP}, {Qt::Key_K, ME_DOWN}, {Qt::Key_J, ME_LEFT}};

    if (e->modifiers() == Qt::ShiftModifier) {
        bool processed = true;
        switch (e->key()) {
        case Qt::Key_Return:
            emit newSiblingPressed();
            break;
        case Qt::Key_Backtab:
            emit movePressed(ME_LEFT);
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
            emit movePressed(ME_RIGHT);
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
            emit switchFocusPressed(ME_TO_BEGIN);
            break;
        case Qt::Key_End:
            emit switchFocusPressed(ME_TO_END);
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
                emit movePressed(ME_UP);
            else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_K)
                emit movePressed(ME_DOWN);
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
                emit movePressed(ME_RIGHT);
            } else {
                processed = false;
            }
        }
            break;
        case Qt::Key_PageUp:
            emit switchFocusPressed(ME_PAGE_UP);
            break;
        case Qt::Key_PageDown:
            emit switchFocusPressed(ME_PAGE_DOWN);
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

ApplicationTask::ApplicationTask(uint64_t id)
    : id(id)
{

}

ApplicationTask::~ApplicationTask()
{

}

uint64_t ApplicationTask::getId()
{
    return id;
}

ReadServerItems::ReadServerItems(uint64_t id, LogControl *control, uint count)
    : ApplicationTask(id)
    , remainintCount(0)
    , control(control)
{
    items[0] = new LogItem(control, nullptr, 0);
    items[0]->setId(0);
}

bool ReadServerItems::process(void *data)
{
    ServerItemData *serverItemData = (ServerItemData *)data;
    LogItem *item = items[serverItemData->itemId];
    assert(item);

    qDebug() << "id: " << serverItemData->itemId << " parent: " << serverItemData->parentId;

    item->setText(serverItemData->text);
    remainintCount--;
    qDebug() << "count: " << remainintCount;
    return remainintCount == 0;
}

void ReadServerItems::processChildren(uint64_t parentId, uint64_t *ids, uint count)
{
    LogItem *parent = items[parentId];
    assert(parent);
    remainintCount += count;
    for (int i = 0; i < count; i++) {
        LogItem *item = new LogItem(control, parent, ids[i]);
        parent->addAsChild(item);
        items[ids[i]] = item;
    }
}

LogItem *ReadServerItems::getRootItem()
{
    return items[0];
}

ApplicationControl::ApplicationControl(LogAppServer *server)
    : server(server)
    , state(AS_START)
    , readAllItemsTask(nullptr)
{
}

QString getRandomFileName(size_t length)
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    QTime currentTime = QTime::currentTime();
    qsrand(currentTime.hour() + currentTime.second() + currentTime.minute() + currentTime.msec());
    QString randomString;
    for(int i=0; i<length; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

bool ApplicationControl::createNewDocument(QString name, QString fullFileName, DocumentType type)
{
    LogControl *newDoc = nullptr;
    if (type == DT_LOCAL || type == DT_CACHED) {
        QFile file(fullFileName);
        if (file.exists()) {
            qDebug() << "error: file exists";
            return false;
        }

        file.open(QIODevice::ReadWrite);
        if (!file.isOpen())
            return false;
        file.close();

        DB *db = new XmlDB(fullFileName);
        newDoc = new LogControl(db, name, LogControl::getNextDocId());
    }
    if (type == DT_REMOTE || type == DT_CACHED) {
        if (!newDoc)
            newDoc = new LogControl(nullptr, name, LogControl::getNextDocId());
//        server->createDocument();
        RemoteDB *rdb = new RemoteDB(server, newDoc);
        newDoc->setServerDB(rdb, type);
    }
    if (newDoc) {
        newDoc->loadData();
        return true;
    }
    return false;

}

void ApplicationControl::start()
{
}

