#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <ui_mainwindow.h>

#include "cmd_executor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QPointer<CmdExecutor> cmdExecutor;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sendCommand(QString cmd);    // send command to command receiver

private slots:
    void on_cmdLineEdit_editingFinished();
    void clearAllDisplays() {
        ui->monitorDisplay->clear();
        ui->breakPointsDisplay->clear();
        ui->treeDisplay->clear();
        ui->cmdLineEdit->clear();
        ui->CodeDisplay->clear();
        ui->textBrowser->clear(); // 运行结果
    }
private:
    Ui::MainWindow *ui;

    void setUIForDebugMode();
    void setUIExitDebugMode();
    void openFileDialog();


};
#endif // MAINWINDOW_H
