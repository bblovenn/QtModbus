#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTabWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupMainTabs();
    int pollingIntervalMs = 1000;
    int pollingStartAddress = 0;
    int pollingCount = 4;
    bool modbusConnected = false;

private:
    Ui::MainWindow *ui;
    QTabWidget *tabs = nullptr;
};

#endif // MAINWINDOW_H
