#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>
#include <QShortcut>

#include "logtextedit.h"
#include "applicationcontrol.h"
#include "logappserver.h"

void MainWindow::addDocumentToList(ClientDocument *doc)
{

    LogAppServer *server = doc->getRemoteDB()->getServer();

    QTreeWidgetItem *parentItem = serverItems[server->getName()];
    if (parentItem == nullptr) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(0);
        treeItem->setText(0, server->getName());
        ui->treeWidget->addTopLevelItem(treeItem);
        serverItems[server->getName()] = treeItem;
        parentItem = treeItem;
        parentItem->setExpanded(true);
    }

    QTreeWidgetItem *treeItem = new QTreeWidgetItem(0);
    treeItem->setText(0, getNameForDoc(doc));
    treeItem->setData(0, Qt::UserRole, QVariant(qulonglong(doc->getId())));
    parentItem->addChild(treeItem);

    idsToDocs[doc->getId()] = doc;
    connect(doc, SIGNAL(docModifiedChanged(bool)), this, SLOT(onDocModifiedChanged(bool)));
}

QString MainWindow::getNameForDoc(const ClientDocument *doc)
{
    static std::map<DocumentType, QString> DT_TO_STRING = {{DT_CACHED, "shared"}, {DT_LOCAL, "local"}, {DT_REMOTE, "remote"}};
    QString docType = DT_TO_STRING[doc->getType()];
    QString name = doc->getName() + (doc->getModified()?"*":"");
    return name;
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
    QAction *settingsAct = new QAction(tr("&Settings"), this);
    ui->mainToolBar->addAction(settingsAct);

    ui->newDocWidget->setVisible(false);

    connect(ui->docOkButton, SIGNAL(clicked(bool)), this, SLOT(newDocButtonClicked()));
    connect(ui->docCancelButton, SIGNAL(clicked(bool)), this, SLOT(newDocButtonClicked()));
    connect(newAct, SIGNAL(triggered(bool)), this, SLOT(createDocument()));
    connect(settingsAct, SIGNAL(triggered(bool)), this, SLOT(showSettingsWindow()));
//    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onDocumentSelected(QListWidgetItem*)));
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onDocumentTreeItemChanged(QTreeWidgetItem*)));

    guiControl = new GuiControl(ui->documentsTab);
    appControl = new ApplicationControl(guiControl);

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), appControl, SLOT(search(QString)));
    connect(appControl, SIGNAL(createdNewDocument(ClientDocument*)), this, SLOT(onNewDocument(ClientDocument*)));
    connect(appControl, SIGNAL(setConnectionStatus(QString)), this, SLOT(onConnectionStatusChanged(QString)));
    connect(appControl, SIGNAL(documentAdded(ClientDocument*)), this, SLOT(onNewDocument(ClientDocument*)));

    serverStatusLabel = new QLabel();
    ui->statusBar->addWidget(serverStatusLabel);

    ui->documentsTab->setTabsClosable(true);

    appControl->start();
}

MainWindow::~MainWindow()
{
    delete appControl;
    delete guiControl;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    //e->ignore();
}

void MainWindow::onDocumentSelected(QListWidgetItem* item)
{
    uint64_t id = item->data(Qt::UserRole).toUInt();
    auto it = idsToDocs.find(id);
    if (it != idsToDocs.end()) {
        guiControl->setCurrentDocument((*it).second);
        appControl->setCurrentDocument((*it).second);
    } else {
        qDebug() << "document with id: " << id << " not found";
    }
}

void MainWindow::onDocumentTreeItemChanged(QTreeWidgetItem *item)
{
    uint64_t id = item->data(0, Qt::UserRole).toUInt();
    auto it = idsToDocs.find(id);
    if (it != idsToDocs.end()) {
        guiControl->setCurrentDocument((*it).second);
        appControl->setCurrentDocument((*it).second);
    } else {
        qDebug() << "document with id: " << id << " not found";
    }
}

void MainWindow::createDocument()
{
    ui->newDocWidget->setVisible(true);
    ui->docNameLine->setFocus();
}

void MainWindow::newDocButtonClicked()
{
    if (sender() == ui->docOkButton) {
        DocumentType type = (ui->docLocalBox->isChecked())?(ui->docRemoteBox->isChecked()?DT_CACHED:DT_LOCAL):DT_REMOTE;
        QString name = ui->docNameLine->text();
        appControl->createNewDocument(name, type);
    }
    ui->newDocWidget->setVisible(false);
}

void MainWindow::onNewDocument(ClientDocument *doc)
{
    auto it = idsToDocs.find(doc->getId());
    if (it != idsToDocs.end()) {
        qDebug() << "error: document with this id already exists";
        return;
    }
    addDocumentToList(doc);
}

void MainWindow::onConnectionStatusChanged(QString status)
{
    serverStatusLabel->setText(status);
}

void MainWindow::onDocModifiedChanged(bool modified)
{
    Q_UNUSED(modified);
    ClientDocument *doc = static_cast<ClientDocument *>(sender());
    if (doc) {
//        for (int i = 0; i <  ui->listWidget->count(); i++) {
//            QListWidgetItem *item = ui->listWidget->item(i);
//            if (item->data(Qt::UserRole).toULongLong() == doc->getId()) {
//                item->setText(getNameForDoc(doc));
//            }
//        }
    }
}

void MainWindow::showSettingsWindow()
{
    settingsWindow.show();
}
