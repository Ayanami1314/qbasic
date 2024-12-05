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
    RUN,
    STOP,
    CLEAR,
    RESUME,
    DEBUG
};
static inline QString cmd2Str(Command cmd) {
    switch (cmd) {
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
    default:
        return "UNKNOWN";
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
signals:
    void sendOutput(std::string outputs);
    void sendError(std::string error);
private slots:
    void receiveCmd(Command cmd);
    void receiveInput(std::string input) {
        interpreter->input(input);
    }
    void receiveOutput(std::string output){
        emit sendOutput(output);
    }
    void receiveError(std::string error) {
        emit sendError(error);
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
    void handleCmdDebug();
    void handleCmdRun();
    void handleCmdClear();
    void handleCmdResume();
    void handleCmdStop();
};



#endif //CMD_EXECUTOR_H
