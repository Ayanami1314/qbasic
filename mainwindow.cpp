#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setUIExitDebugMode();
    cmdExecutor = new CmdExecutor();
    connect(ui->btnDebugMode, &QPushButton::clicked, this, &MainWindow::setUIForDebugMode);
    connect(ui->btnExitDebugMode, &QPushButton::clicked, this, &MainWindow::setUIExitDebugMode);
    connect(ui->btnClearCode, &QPushButton::clicked, this, &MainWindow::clearAllDisplays);
    connect(this, &MainWindow::sendCommand, cmdExecutor, &CmdExecutor::receiveCmd);
    connect(cmdExecutor, &CmdExecutor::sendOutput, ui->textBrowser, &QTextBrowser::append);
    // stderr = stdout
    connect(cmdExecutor, &CmdExecutor::sendError, ui->textBrowser, &QTextBrowser::append);
    // TODO: input area
    // TODO: load button click file choose
    connect(ui->btnLoadCode, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    connect(ui->btnRunCode, &QPushButton::clicked, [this](){
        cmdExecutor->runCmd(Command::RUN, {});
    });
    connect(ui->btnDebugResume, &QPushButton::clicked, [this](){
        cmdExecutor->runCmd(Command::RESUME, {});
    });
    connect(ui->btnDebugMode, &QPushButton::clicked, [this](){
        cmdExecutor->runCmd(Command::DEBUG, {});
    });
    connect(ui->btnExitDebugMode, &QPushButton::clicked, [this](){
            cmdExecutor->runCmd(Command::STOP, {});
    });
}

MainWindow::~MainWindow()
{
    delete ui;
    cmdExecutor->deleteLater();
}
void MainWindow::openFileDialog() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        // Handle the selected file
        qDebug() << "Selected file: " << fileName.toUtf8();

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            ui->CodeDisplay->setPlainText(fileContent);
            file.close();
            cmdExecutor->runCmd(Command::LOAD, {fileName.toStdString()});
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file"));
        }
    }
}
void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text();
    ui->cmdLineEdit->setText("");
    // TODO: support edit
    // ui->CodeDisplay->append(cmd);
    emit sendCommand(cmd);

}

void MainWindow::setUIForDebugMode(){
    ui->btnClearCode->setVisible(false);
    ui->btnLoadCode->setVisible(false);
    ui->btnDebugMode->setVisible(false);

    ui->btnExitDebugMode->setVisible(true);
    ui->btnDebugResume->setVisible(true);

    ui->labelSyntaxTree->setVisible(false);
    ui->treeDisplay->setVisible(false);

    ui->labelMonitor->setVisible(true);
    ui->monitorDisplay->setVisible(true);
    ui->labelBreakPoints->setVisible(true);
    ui->breakPointsDisplay->setVisible(true);
}

void MainWindow::setUIExitDebugMode(){
    ui->btnClearCode->setVisible(true);
    ui->btnLoadCode->setVisible(true);
    ui->btnDebugMode->setVisible(true);

    ui->btnExitDebugMode->setVisible(false);
    ui->btnDebugResume->setVisible(false);

    ui->labelSyntaxTree->setVisible(true);
    ui->treeDisplay->setVisible(true);

    ui->labelMonitor->setVisible(false);
    ui->monitorDisplay->setVisible(false);
    ui->labelBreakPoints->setVisible(false);
    ui->breakPointsDisplay->setVisible(false);
}
