#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->labelInputRequired->setVisible(false);
    setUIExitDebugMode();
    cmdExecutor = new CmdExecutor();
    connect(ui->btnDebugMode, &QPushButton::clicked, this, [this]() {
        setUIForDebugMode();
        cmdExecutor->setMode(ProgramMode::DEBUG);
    });
    connect(ui->btnExitDebugMode, &QPushButton::clicked, this, [this]() {
        setUIExitDebugMode();
        cmdExecutor->setMode(ProgramMode::NORMAL);
    });
    connect(ui->btnClearCode, &QPushButton::clicked, this, &MainWindow::clearAllDisplays);
    connect(this, &MainWindow::sendCommand, cmdExecutor, &CmdExecutor::receiveCmd);
    connect(cmdExecutor, &CmdExecutor::sendOutput, ui->textBrowser, &QTextBrowser::append);
    // stderr = stdout
    connect(cmdExecutor, &CmdExecutor::sendError, ui->textBrowser, &QTextBrowser::append);

    connect(ui->btnLoadCode, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    connect(ui->btnRunCode, &QPushButton::clicked, [this](){
        // 此处必须解耦, 否则可能卡在input等,相当于分离的线程
        sendCommand("RUN");
        if(cmdExecutor->getMode() == ProgramMode::DEBUG) {
             showEnv();
        }
    });
    connect(ui->btnDebugResume, &QPushButton::clicked, [this](){
        sendCommand("RESUME");
        if(cmdExecutor->getMode() == ProgramMode::DEBUG) {
            showEnv();
        }
    });
    connect(ui->btnDebugMode, &QPushButton::clicked, [this](){
        sendCommand("DEBUG");
        showEnv();
    });
    connect(ui->btnExitDebugMode, &QPushButton::clicked, [this](){
        sendCommand("STOP");
        showBreakpoints();
    });
    connect(this, &MainWindow::sendInput, cmdExecutor, &CmdExecutor::receiveInput);
    connect(cmdExecutor, &CmdExecutor::sendAST, ui->treeDisplay, &QTextBrowser::append);
    connect(cmdExecutor, &CmdExecutor::waitingForInput, [this](){
        ui->inputLineEdit->setEnabled(true);
        ui->labelInputRequired->setVisible(true);
    });
    connect(cmdExecutor, &CmdExecutor::sendError, [this](QString error){
        QMessageBox::warning(this, tr("Error"), error);
    });
    connect(cmdExecutor, &CmdExecutor::breakpointChanged, this, &MainWindow::showBreakpoints);
}
void MainWindow::showEnv() {
    auto repl = cmdExecutor->getEnv()->getRepl();
    ui->monitorDisplay->clear();
    for(const auto& s: repl) {
        ui->monitorDisplay->append(QString::fromStdString(s));
    }
}
void MainWindow::showBreakpoints() {
    auto breakpoints = cmdExecutor->getBreakpoints();
    ui->breakPointsDisplay->clear();
    for(const auto& line: breakpoints) {
        ui->breakPointsDisplay->append(QString("Breakpoint at: %1").arg(line));
    }
}
void showBreakpoints() {

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
void MainWindow::on_inputLineEdit_editingFinished()
{
    QString input = ui->inputLineEdit->text();
    ui->inputLineEdit->setText("");
    QString inputDisplay = "> " + input;
    ui->textBrowser->append(inputDisplay);
    ui->labelInputRequired->setVisible(false);
    emit sendInput(input);
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
