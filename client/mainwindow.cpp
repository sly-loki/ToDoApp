#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>

#include "logtextedit.h"
#include "logappserver.h"

#define TEST_FILE_NAME "/home/loki/.todo/midterm.xml"
#define DEFAULT_APP_FOLDER_NAME ".todo/test"

void MainWindow::addDocumentToList(LogControl *doc)
{
    QListWidgetItem *item = new QListWidgetItem(doc->getName());

    item->setData(Qt::UserRole, QVariant(qulonglong(doc->getId())));
    ui->listWidget->addItem(item);

    idsToDocs[doc->getId()] = doc;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setCollapsible(1, false);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);

    QAction *newAct = new QAction(tr("&New"), this);
    ui->mainToolBar->addAction(newAct);
    ui->newDocWidget->setVisible(false);

    connect(ui->docOkButton, SIGNAL(clicked(bool)), this, SLOT(newDocButtonClicked()));
    connect(ui->docCancelButton, SIGNAL(clicked(bool)), this, SLOT(newDocButtonClicked()));
//    connect(ui->docNameLine, SIGNAL())
    connect(newAct, SIGNAL(triggered(bool)), this, SLOT(createDocument()));

    QString appDirectoryName = QDir::homePath() + QDir::separator() + DEFAULT_APP_FOLDER_NAME;
    qDebug() << appDirectoryName;
    appDir = QDir(appDirectoryName);
    if (!appDir.exists()) {
        appDir.mkpath(appDirectoryName);
    }

    QStringList filters;
    filters.append("*.xml");
    QStringList fileList = appDir.entryList(filters);
    qDebug() << fileList;

    for (int i = 0; i < fileList.size(); i++) {
        auto s = fileList[i];
        QString fileName = appDir.path() + QDir::separator() + s;

        DB *db = new XmlDB(fileName);
        LogControl *control = new LogControl(db, s, i);
        control->loadData();

        addDocumentToList(control);
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onDocumentSelected(QListWidgetItem*)));

    guiControl = new GuiControl(ui->scrollArea);

    server = new LogAppServer();
    appControl = new ApplicationControl(server);

    connect(server, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(server, SIGNAL(disconnected(QString)), this, SLOT(onServerDisconnected(QString)));
    connect(server, SIGNAL(docListReceived(std::vector<std::pair<uint64_t,QString> >)), this, SLOT(onDocListReceived(std::vector<std::pair<uint64_t,QString> >)));
    connect(appControl, SIGNAL(createdNewDocument(LogControl*)), this, SLOT(onNewDocument(LogControl*)));

    connectionTimer.setInterval(5000);
    connectionTimer.setSingleShot(true);
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(serverPooling()));

    serverStatusLabel = new QLabel();
    ui->statusBar->addWidget(serverStatusLabel);
    server->connectToServer();
    serverPooling();

    ui->listWidget->setCurrentItem(ui->listWidget->item(0));

//        appControl->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDocumentSelected(QListWidgetItem* item)
{
    uint64_t id = item->data(Qt::UserRole).toUInt();
    auto it = idsToDocs.find(id);
    if (it != idsToDocs.end()) {
        guiControl->setCurrentDocument((*it).second);
    } else {
        qDebug() << "document with id: " << id << " not found";
    }
}

void MainWindow::createDocument()
{
    ui->newDocWidget->setVisible(true);
}

void MainWindow::newDocButtonClicked()
{
    if (sender() == ui->docOkButton) {
        DocumentType type = (ui->docLocalBox->isChecked())?(ui->docRemoteBox->isChecked()?DT_CACHED:DT_LOCAL):DT_REMOTE;
        QString name = ui->docNameLine->text();
        QString fileName = appDir.path() + QDir::separator() + name + ".xml";
        if (!QFile(fileName).exists()) {
            appControl->createNewDocument(name, fileName, type);
        }
    }
    ui->newDocWidget->setVisible(false);
}

void MainWindow::onNewDocument(LogControl *doc)
{
    auto it = idsToDocs.find(doc->getId());
    if (it != idsToDocs.end()) {
        qDebug() << "error: document with this id already exists";
        return;
    }
    addDocumentToList(doc);
}

void MainWindow::serverPooling()
{
    if (server->getStatus() == SS_CONNECTED) {
        serverStatusLabel->setText("connected");
    } else {
//        if (server->getStatus() == SS_DISCONNECTED)
        server->connectToServer();
        connectionTimer.start();
    }

}

void MainWindow::onServerConnected()
{
    server->getDocList();
}

void MainWindow::onServerDisconnected(QString reason)
{
    serverStatusLabel->setText(reason);
    serverPooling();
}

void MainWindow::onDocListReceived(std::vector<std::pair<uint64_t, QString> > docs)
{
    for (auto s: docs) {
        LogControl *doc = new LogControl(nullptr, s.second, s.first);
        qDebug() << "created remote doc with id: " << s.first;
        RemoteDB *rdb = new RemoteDB(server, doc);
        doc->setServerDB(rdb, DT_REMOTE);
        doc->loadData();

        addDocumentToList(doc);
    }
}
