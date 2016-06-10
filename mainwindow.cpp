#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <memory>

#include "logtextedit.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DB *db = new XmlDB("/home/temp/temp.xml");
    GuiControl *guiControl = new GuiControl(ui->scrollArea);
    LogControl *control = new LogControl(guiControl, db);

}

MainWindow::~MainWindow()
{
    delete ui;
}
