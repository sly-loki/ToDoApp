#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include "logtextedit.h"

#define TEST_FILE_NAME "/tmp/test.xml"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DB *db = new XmlDB(TEST_FILE_NAME);
    GuiControl *guiControl = new GuiControl(ui->scrollArea);
    LogControl *control = new LogControl(db);
    connect(control, SIGNAL(itemAdded(LogItem*)), guiControl, SLOT(addItem(LogItem*)));
    connect(control, SIGNAL(itemDeleted(LogItem*)), guiControl, SLOT(removeItem(LogItem*)));
    connect(control, SIGNAL(itemModified(LogItem*)), guiControl, SLOT(updateItem(LogItem*)));
    connect(control, SIGNAL(itemFocused(LogItem*)), guiControl, SLOT(focusItem(LogItem*)));
    control->loadData();
}

MainWindow::~MainWindow()
{
    delete ui;
}
