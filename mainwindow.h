#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <map>

namespace Ui {
class MainWindow;
}

class LogControl;
class GuiControl;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::map<QString, LogControl *> filesToDocs;
    GuiControl *guiControl;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onDocumentSelected(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
