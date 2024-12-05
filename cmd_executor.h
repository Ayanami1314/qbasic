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
    Interpreter* interpreter;
    Token::Tokenizer* tokenizer;
    Parser *parser;
    Env* env;
private slots:
    void receiveCmd(QString cmd);
public:
    CmdExecutor() {
        tokenizer = new Token::Tokenizer();
        env = new Env();
        parser = new Parser(tokenizer, &(env->symbol_table));
        interpreter = new Interpreter(parser);
    }
    ~CmdExecutor() override {
        // delete interpreter;
        // delete parser;
        // delete tokenizer;
        // delete env;
    }
};



#endif //CMD_EXECUTOR_H
