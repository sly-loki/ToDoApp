#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QDir>

#include <map>

namespace Ui {
class MainWindow;
}

class LogControl;
class GuiControl;
class ApplicationControl;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::map<QString, LogControl *> filesToDocs;
    GuiControl *guiControl;
    ApplicationControl *appControl;
    QDir appDir;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onDocumentSelected(QListWidgetItem *item);
    void createDocument();
    void newDocButtonClicked();
    void onNewDocument(LogControl *doc);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
