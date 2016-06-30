#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>
#include <QDir>

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

    QString appDirectoryName = QDir::homePath() + QDir::separator() + DEFAULT_APP_FOLDER_NAME;
    qDebug() << appDirectoryName;
    QDir appDir(appDirectoryName);
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
        LogControl *control = new LogControl(db);
        control->loadData();

        filesToDocs[s] = control;

        ui->listWidget->addItem(s);
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onDocumentSelected(QListWidgetItem*)));

    DB *db = new XmlDB(TEST_FILE_NAME);
    guiControl = new GuiControl(ui->scrollArea);
    LogControl *control = new LogControl(db);

    LogAppServer *server = new LogAppServer();
    ApplicationControl *appControl = new ApplicationControl(control, server);
    connect(control, SIGNAL(itemAdded(LogItem*)), server, SLOT(addItem(LogItem*)));
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
