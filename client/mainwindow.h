#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QDir>
#include <QTimer>
#include <QLabel>

#include <map>

#include "settingswindow.h"

namespace Ui {
class MainWindow;
}

class ClientDocument;
class GuiControl;
class ApplicationControl;
class LogAppServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    std::map<uint64_t, ClientDocument *> idsToDocs;
    GuiControl *guiControl;
    ApplicationControl *appControl;
    QLabel *serverStatusLabel;

    SettingsWindow settingsWindow;

    void addDocumentToList(ClientDocument *doc);
    QString getNameForDoc(const ClientDocument *doc);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e) override;

protected slots:
    void onDocumentSelected(QListWidgetItem *item);
    void createDocument();
    void newDocButtonClicked();

    void onNewDocument(ClientDocument *doc);
    void onConnectionStatusChanged(QString status);
    void onDocModifiedChanged(bool modified);

    void showSettingsWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
