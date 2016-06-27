#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include <QSplitter>
#include <QListWidget>

#include "logtextedit.h"
#include "logappserver.h"

#define TEST_FILE_NAME "/home/loki/.todo/test.xml"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setCollapsible(1, false);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);

    DB *db = new XmlDB(TEST_FILE_NAME);
    GuiControl *guiControl = new GuiControl(ui->scrollArea);
    LogControl *control = new LogControl(db);
    connect(control, SIGNAL(itemAdded(LogItem*)), guiControl, SLOT(addItem(LogItem*)));
    connect(control, SIGNAL(itemDeleted(LogItem*)), guiControl, SLOT(removeItem(LogItem*)));
    connect(control, SIGNAL(itemModified(LogItem*)), guiControl, SLOT(updateItem(LogItem*)));
    connect(control, SIGNAL(itemFocused(LogItem*)), guiControl, SLOT(focusItem(LogItem*)));
    connect(control, SIGNAL(itemDoneChanged(LogItem*)), guiControl, SLOT(setItemDone(LogItem*)));

    connect(guiControl, SIGNAL(itemDoneChanged(LogItem*,bool)), control, SLOT(setItemDone(LogItem*,bool)));

    LogAppServer *server = new LogAppServer();
    ApplicationControl *appControl = new ApplicationControl(control, server);
    connect(control, SIGNAL(itemAdded(LogItem*)), server, SLOT(addItem(LogItem*)));
    control->loadData();
//    server->connectToServer();
//    server->getItemList();
//    appControl->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
