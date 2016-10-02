#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>

#include "logtextedit.h"
#include "applicationcontrol.h"
#include "logappserver.h"

void MainWindow::addDocumentToList(LogControl *doc)
{
    static std::map<DocumentType, QString> DT_TO_STRING = {{DT_CACHED, "shared"}, {DT_LOCAL, "local"}, {DT_REMOTE, "remote"}};
    QString docType = DT_TO_STRING[doc->getType()];

    QListWidgetItem *item = new QListWidgetItem(doc->getName() + " (" + docType + ")");

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
    connect(newAct, SIGNAL(triggered(bool)), this, SLOT(createDocument()));
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onDocumentSelected(QListWidgetItem*)));

    guiControl = new GuiControl(ui->scrollArea);
    appControl = new ApplicationControl();

    connect(appControl, SIGNAL(createdNewDocument(LogControl*)), this, SLOT(onNewDocument(LogControl*)));
    connect(appControl, SIGNAL(setConnectionStatus(QString)), this, SLOT(onConnectionStatusChanged(QString)));
    connect(appControl, SIGNAL(documentAdded(LogControl*)), this, SLOT(onNewDocument(LogControl*)));


    serverStatusLabel = new QLabel();
    ui->statusBar->addWidget(serverStatusLabel);

    appControl->start();
}

MainWindow::~MainWindow()
{
    delete appControl;
    delete guiControl;
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

void MainWindow::onNewDocument(LogControl *doc)
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
