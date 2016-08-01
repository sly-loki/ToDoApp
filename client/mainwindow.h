#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QDir>
#include <QTimer>
#include <QLabel>

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

    std::map<uint64_t, LogControl *> idsToDocs;
    GuiControl *guiControl;
    ApplicationControl *appControl;
    LogAppServer *server;
    QDir appDir;
    QTimer connectionTimer;
    QLabel *serverStatusLabel;

    void addDocumentToList(LogControl *doc);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onDocumentSelected(QListWidgetItem *item);
    void createDocument();
    void newDocButtonClicked();
    void onNewDocument(LogControl *doc);
    void serverPooling();

    void onServerConnected();
    void onServerDisconnected(QString reason);
    void onDocListReceived(std::vector<std::pair<uint64_t, QString>> docs);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
