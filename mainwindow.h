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
class LogAppServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::map<QString, LogControl *> filesToDocs;
    GuiControl *guiControl;
    ApplicationControl *appControl;
    LogAppServer *server;
    QDir appDir;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onDocumentSelected(QListWidgetItem *item);
    void createDocument();
    void newDocButtonClicked();
    void onNewDocument(LogControl *doc);

    void onServerConnected();
    void onDocListReceived(std::vector<QString> docs);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
