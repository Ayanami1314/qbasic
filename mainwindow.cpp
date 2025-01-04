#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
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
    connect(ui->btnClearCode, &QPushButton::clicked, this, [this](){
        clearAllDisplays();
        cmdExecutor->runCmd(Command::STOP, {});
        cmdExecutor->runCmd(Command::CLEAR, {});
    });
    connect(this, &MainWindow::sendCommand, cmdExecutor, &CmdExecutor::receiveCmd);
    connect(cmdExecutor, &CmdExecutor::sendOutput, ui->textBrowser, &QTextBrowser::append);
    // stderr = stdout
    connect(cmdExecutor, &CmdExecutor::sendError, ui->textBrowser, &QTextBrowser::append);

    connect(ui->btnLoadCode, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    connect(ui->btnRunCode, &QPushButton::clicked, [this](){
        // 此处必须解耦, 否则可能卡在input等,相当于分离的线程
        auto text =  ui->CodeDisplay->toPlainText();
        clearAllDisplays();
        ui->CodeDisplay->setPlainText(text);
        sendCommand(cmd2Str(Command::RUN));
        if(cmdExecutor->getMode() == ProgramMode::DEBUG) {
             showEnv();
        }
    });
    connect(ui->btnDebugResume, &QPushButton::clicked, [this](){
        sendCommand(cmd2Str(Command::RESUME));
        if(cmdExecutor->getMode() == ProgramMode::DEBUG) {
            showEnv();
        }
    });
    connect(ui->btnDebugMode, &QPushButton::clicked, [this](){
        sendCommand(cmd2Str(Command::DEBUG));
        showEnv();
    });
    connect(ui->btnExitDebugMode, &QPushButton::clicked, [this](){
        sendCommand(cmd2Str(Command::STOP));
        showBreakpoints();
    });
    connect(this, &MainWindow::sendInput, cmdExecutor, &CmdExecutor::receiveInput);
    connect(cmdExecutor, &CmdExecutor::sendAST, ui->treeDisplay, &QTextBrowser::append);
    connect(cmdExecutor, &CmdExecutor::waitingForInput, [this](){
        ui->inputLineEdit->setEnabled(true);
        ui->labelInputRequired->setVisible(true);
    });
    connect(cmdExecutor, &CmdExecutor::sendError, [this](QString error){
        showError(error);
    });
    connect(cmdExecutor, &CmdExecutor::breakpointChanged, this, &MainWindow::showBreakpoints);
    connect(ui->btnHelp, &QPushButton::clicked, this, &MainWindow::openHelp);
}
void MainWindow::showEnv() {
    auto repl = cmdExecutor->getEnv()->getRepl();
    ui->monitorDisplay->clear();
    for(const auto& s: repl) {
        ui->monitorDisplay->append(QString::fromStdString(s));
    }
}
void MainWindow::showError(QString msg) {
    QMessageBox::warning(this, tr("Error"), msg);
}
void MainWindow::showBreakpoints() {
    auto breakpoints = cmdExecutor->getBreakpoints();
    ui->breakPointsDisplay->clear();
    for(const auto& line: breakpoints) {
        ui->breakPointsDisplay->append(QString("Breakpoint at: %1").arg(line));
    }
}
void MainWindow::openHelp() {
    QFile file("./assets/help-zh.md");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString helpContent = in.readAll();
        QMessageBox::information(this, tr("Help"), helpContent);
        file.close();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open help file"));
    }
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
        saveFile();
    }
}
void MainWindow::saveFile(QString filepath) {
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->CodeDisplay->toPlainText();
        file.close();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save file"));
    }
}
void MainWindow::saveFile() {
    auto curFileName = cmdExecutor->getChoosedFile();
    if(std::filesystem::exists(curFileName)) {
        saveFile(QString::fromStdString(curFileName));
        return;
    }
    // save as
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        saveFile(fileName);
        cmdExecutor->runCmd(Command::LOAD, {fileName.toStdString()});
    }
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
            // QTextStream in(&file);
            // QString fileContent = in.readAll();
            // ui->CodeDisplay->setPlainText(fileContent);
            // file.close();
            // MAKE DOC HAPPY
            clearAllDisplays();
            cmdExecutor->runCmd(Command::LOAD, {fileName.toStdString()});
            auto sortedSrc = cmdExecutor->getInterpreter()->getSortedSrc();
            QString code;
            for(const auto&[line_no, line]: sortedSrc.lines) {
                code += QString::number(line_no) + " " + QString::fromStdString(line) + "\n";
            }
            ui->CodeDisplay->setPlainText(code);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file"));
        }
    }
}
void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text();
    ui->cmdLineEdit->setText("");
    // built in cmd
    if(cmd == "HELP") {
        openHelp();
        return;
    }
    if(cmd == "QUIT") {
        close();
        return;
    }
    // edit

    int line_no;
    try {
        std::istringstream iss(cmd.toStdString());
        iss >> line_no;
        if (iss.fail()) {
            // not a number
            print("send cmd: {}\n", cmd.toStdString());
            emit sendCommand(cmd);
            return;
        }
        print("cmd insert line: {}\n", line_no);
        // line_no 开头，是编辑
        if(line_no < 0) {
            QMessageBox::warning(this, tr("Error"), tr("不能编辑负数行"));
            return;
        }
        auto text = ui->CodeDisplay->toPlainText();
        bool inserted = false;
        QStringList lines = text.split("\n");

        for(int i = 0; i < lines.size(); i++) {
            int cur_line;
            iss.clear();
            iss.str(lines[i].toStdString());
            iss >> cur_line;
            if(cur_line > line_no) {
                lines.insert(i, cmd);
                inserted = true;
                break;
            }
            if (cur_line == line_no) {
                lines[i] = cmd;
                inserted = true;
                break;
            }
        }
        if(!inserted) {
            lines.append(cmd);
        }
        // 插入也视为CMD
        vector<std::string> new_lines;
        for (const auto & line : lines) {
            new_lines.push_back(line.toStdString());
        }
        bool success = cmdExecutor->reloadProgram(new_lines);
        if(success) {
            ui->CodeDisplay->setPlainText(lines.join("\n"));
        }
        return;

    } catch (std::exception& e) {

    }

    print("send cmd: {}\n", cmd.toStdString());
    emit sendCommand(cmd);
}

void MainWindow::on_inputLineEdit_editingFinished()
{
    static const QString input_hint = "?"; // used to be ">";
    QString input = ui->inputLineEdit->text();
    ui->inputLineEdit->setText("");
    QString inputDisplay = input_hint + " " + input;
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
