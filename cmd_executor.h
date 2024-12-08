//
// Created by ayanami on 12/5/24.
//

#ifndef CMD_EXECUTOR_H
#define CMD_EXECUTOR_H


#include <QObject>

#include "interpreter.h"
#include "parser.h"
#include "tokenizer.h"


enum class Command {
    CHANGE_MODE, // change mode + mode name "DEBUG" "NORMAL" "DEV"
    LOAD,
    RUN,
    STOP,
    CLEAR,
    RESUME,
    DEBUG,
    ADD_BREAKPOINT,
    REMOVE_BREAKPOINT,
    UNKNOWN
};
static inline QString cmd2Str(Command cmd) {
    switch (cmd) {
    case Command::CHANGE_MODE:
        return "CHANGE_MODE";
    case Command::LOAD:
        return "LOAD";
    case Command::RUN:
        return "RUN";
    case Command::STOP:
        return "STOP";
    case Command::CLEAR:
        return "CLEAR";
    case Command::RESUME:
        return "RESUME";
    case Command::DEBUG:
        return "DEBUG";
    case Command::ADD_BREAKPOINT:
        return "ADD_BREAKPOINT";
    case Command::REMOVE_BREAKPOINT:
        return "REMOVE_BREAKPOINT";
    default:
        return "UNKNOWN";
    }
}
static inline Command str2Cmd(const QString& cmd) {
    if(cmd == "LOAD") {
        return Command::LOAD;
    } else if(cmd == "RUN") {
        return Command::RUN;
    } else if(cmd == "STOP") {
        return Command::STOP;
    } else if(cmd == "CLEAR") {
        return Command::CLEAR;
    } else if(cmd == "RESUME") {
        return Command::RESUME;
    } else if(cmd == "DEBUG") {
        return Command::DEBUG;
    } else if(cmd == "ADD_BREAKPOINT") {
        return Command::ADD_BREAKPOINT;
    } else if(cmd == "REMOVE_BREAKPOINT") {
        return Command::REMOVE_BREAKPOINT;
    } else if(cmd == "CHANGE_MODE") {
        return Command::CHANGE_MODE;
    }
    else  {
        return Command::UNKNOWN;
    }
}

// A middleware class that connects the GUI and the interpreter
class CmdExecutor: public QObject{
    Q_OBJECT
private:
    std::shared_ptr<Interpreter> interpreter;
    std::shared_ptr<Token::Tokenizer> tokenizer;
    std::shared_ptr<Parser> parser;
    std::shared_ptr<Env> env;
    std::filesystem::path choosed_file {};
    ProgramMode mode = ProgramMode::NORMAL;
signals:
    void sendOutput(QString outputs);
    void sendError(QString error);
public slots:
    // cmd: split by space
    void receiveCmd(QString cmd) {
        QStringList l = cmd.split(" ");
        if(l.empty()) {
            print("Empty command\n");
            return;
        }
        auto cmdType = str2Cmd(l[0]);
        if(cmdType == Command::UNKNOWN) {
            print("Unknown command: {}\n", l[0].toStdString());
            return;
        }
        l.pop_front();
        std::vector<std::string> argv;
        for(const auto& s: l) {
            argv.push_back(s.toStdString());
        }
        runCmd(cmdType, argv);
    }
    void receiveInput(std::string input) {
        interpreter->input(input);
    }
    void chooseFile(std::string file) {
        choosed_file = file;
    }
    void receiveOutput(std::string output) {
        emit sendOutput(QString::fromStdString(output));
    }
    void receiveError(std::string error) {
        emit sendError(QString::fromStdString(error));
    }

public:
    CmdExecutor() {
        tokenizer = std::make_shared<Token::Tokenizer>();
        auto symbol_table = std::make_shared<SymbolTable>();
        env = std::make_shared<Env>(symbol_table);
        parser = std::make_shared<Parser>(tokenizer, symbol_table);
        interpreter = std::make_shared<Interpreter>(parser, env);
        connect(interpreter->getOutputStream(), &MockOutputStream::sendOutput,
            this, &CmdExecutor::receiveOutput);
    }
    ~CmdExecutor() override = default;
    std::string getChoosedFile() {
        return choosed_file.string();
    }
    void runCmd(Command cmd, const std::vector<std::string>& argv);
    void handleCmdChangeMode(const vector<std::string>& argv);
    void handleCmdLoad(const vector<std::string>& argv);
    void handleCmdDebug(const vector<std::string>& argv);
    void handleCmdRun(const vector<std::string>& argv);
    void handleCmdClear(const vector<std::string>& argv);
    void handleCmdResume(const vector<std::string>& argv);
    void handleCmdStop(const vector<std::string>& argv);
    void handleCmdAddBreakpoint(const vector<std::string>& argv);
    void handleCmdRemoveBreakpoint(const vector<std::string> &argv);
};



#endif //CMD_EXECUTOR_H
