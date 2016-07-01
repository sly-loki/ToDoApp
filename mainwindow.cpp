#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>

#include "logtextedit.h"
#include "logappserver.h"

#define TEST_FILE_NAME "/home/loki/.todo/midterm.xml"
#define DEFAULT_APP_FOLDER_NAME ".todo"

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

    for (auto s: fileList) {
        QString fileName = appDir.path() + QDir::separator() + s;

        DB *db = new XmlDB(fileName);
        LogControl *control = new LogControl(db, s);
        control->loadData();

        filesToDocs[s] = control;

        ui->listWidget->addItem(s);
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onDocumentSelected(QListWidgetItem*)));

    DB *db = new XmlDB(TEST_FILE_NAME);
    guiControl = new GuiControl(ui->scrollArea);

    LogAppServer *server = new LogAppServer();
    appControl = new ApplicationControl(server);
//    connect(control, SIGNAL(itemAdded(LogItem*)), server, SLOT(addItem(LogItem*)));
    connect(appControl, SIGNAL(createdNewDocument(LogControl*)), this, SLOT(onNewDocument(LogControl*)));
#define LOCALMOD
#ifdef LOCALMOD

//    guiControl->setCurrentDocument(control);
    ui->listWidget->setCurrentItem(ui->listWidget->item(0));
#else
        server->connectToServer();
        appControl->start();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDocumentSelected(QListWidgetItem* item)
{
    QString name = item->text();
    auto it = filesToDocs.find(name);
    if (it != filesToDocs.end()) {
        guiControl->setCurrentDocument((*it).second);
    } else {
        qDebug() << name << " not found";
    }
}

void MainWindow::createDocument()
{
    ui->newDocWidget->setVisible(true);
}

void MainWindow::newDocButtonClicked()
{
    if (sender() == ui->docOkButton) {
        QString name = ui->docNameLine->text();
        QString fileName = appDir.path() + QDir::separator() + name + ".xml";
        if (!QFile(fileName).exists()) {
            appControl->createNewDocument(name, fileName);
        }
    }
    ui->newDocWidget->setVisible(false);
}

void MainWindow::onNewDocument(LogControl *doc)
{
    filesToDocs[doc->getName()] = doc;

    ui->listWidget->addItem(doc->getName());
}
