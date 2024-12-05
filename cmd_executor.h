//
// Created by ayanami on 12/5/24.
//

#ifndef CMD_EXECUTOR_H
#define CMD_EXECUTOR_H


#include <QObject>

#include "interpreter.h"
#include "parser.h"
#include "tokenizer.h"
// A middleware class that connects the GUI and the interpreter
class CmdExecutor: public QObject{
    Q_OBJECT
private:
    std::shared_ptr<Interpreter> interpreter;
    std::shared_ptr<Token::Tokenizer> tokenizer;
    std::shared_ptr<Parser> parser;
    std::shared_ptr<Env> env;
private slots:
    void receiveCmd(QString cmd);
public:
    CmdExecutor() {
        tokenizer = std::make_shared<Token::Tokenizer>();
        auto symbol_table = std::make_shared<SymbolTable>();
        env = std::make_shared<Env>(symbol_table);
        parser = std::make_shared<Parser>(tokenizer, symbol_table);
        interpreter = std::make_shared<Interpreter>(parser, env);
    }
    ~CmdExecutor() override = default;
};



#endif //CMD_EXECUTOR_H
