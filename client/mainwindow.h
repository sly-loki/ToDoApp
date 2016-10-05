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
    QLabel *serverStatusLabel;

    void addDocumentToList(LogControl *doc);
    QString getNameForDoc(const LogControl *doc);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e) override;

protected slots:
    void onDocumentSelected(QListWidgetItem *item);
    void createDocument();
    void newDocButtonClicked();

    void onNewDocument(LogControl *doc);
    void onConnectionStatusChanged(QString status);
    void onDocModifiedChanged(bool modified);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
