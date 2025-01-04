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
    // cmdline interpret
    LET,
    PRINT,
    INPUT,
    UNKNOWN
};
static bool isAnonymousProgramCmd(Command cmd) {
    return cmd == Command::LET || cmd == Command::PRINT || cmd == Command::INPUT;
}
static inline QString cmd2Str(Command cmd) {
    return QString::fromStdString(std::string(NAMEOF_ENUM(cmd)));
}
// static inline QString cmd2Str(Command cmd) {
//     switch (cmd) {
//     case Command::CHANGE_MODE:
//         return "CHANGE_MODE";
//     case Command::LOAD:
//         return "LOAD";
//     case Command::RUN:
//         return "RUN";
//     case Command::STOP:
//         return "STOP";
//     case Command::CLEAR:
//         return "CLEAR";
//     case Command::RESUME:
//         return "RESUME";
//     case Command::DEBUG:
//         return "DEBUG";
//     case Command::ADD_BREAKPOINT:
//         return "ADD_BREAKPOINT";
//     case Command::REMOVE_BREAKPOINT:
//         return "REMOVE_BREAKPOINT";
//     case Command::LET:
//         return "LET";
//     case Command::PRINT:
//         return "PRINT";
//     case Command::INPUT:
//         return "INPUT";
//     default:
//         return "UNKNOWN";
//     }
// }
static inline Command str2Cmd(const std::string& cmd) {
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
    } else if(cmd == "ADD_BREAKPOINT" || cmd == "ADD") {
        return Command::ADD_BREAKPOINT;
    } else if(cmd == "REMOVE_BREAKPOINT" || cmd == "REMOVE" ||
        cmd == "DEL" || cmd == "DELETE" || cmd == "DELETE_BREAKPOINT") {
        return Command::REMOVE_BREAKPOINT;
    } else if(cmd == "CHANGE_MODE") {
        return Command::CHANGE_MODE;
    } else if (cmd == "LET") {
        return Command::LET;
    } else if (cmd == "PRINT") {
        return Command::PRINT;
    } else if (cmd == "INPUT") {
        return Command::INPUT;
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
    std::shared_ptr<Interpreter> oneLineInterpreter;
    std::shared_ptr<Token::Tokenizer> tokenizer;
    std::shared_ptr<Parser> parser;
    std::shared_ptr<Env> env;
    std::filesystem::path choosed_file {};
    ProgramMode mode = ProgramMode::NORMAL;
signals:
    void sendOutput(QString outputs);
    void sendError(QString error);
    void sendAST(QString ast);
    void waitingForInput();
    void breakpointChanged();
public slots:
    // cmd: split by space
    void receiveCmd(QString cmd) {
        print("Receive command: {}\n", cmd.toStdString());
        auto cmd_str = cmd.toStdString();
        auto l = util::split_by_space(cmd_str);

        if(l.empty()) {
            print("Empty command\n");
            return;
        }
        auto cmdType = str2Cmd(l[0]);
        if(cmdType == Command::UNKNOWN) {
            print("Unknown command: {}\n", l[0]);
            return;
        }

        if(isAnonymousProgramCmd(cmdType)) {
            try {
                handleAnonymousProgramCmd(cmd_str);
            } catch (std::exception& e) {
                print("Failed to run command: {}\n", e.what());
                emit sendError(QString::fromStdString(e.what()));
            }
            return;
        }
        l.erase(l.begin());
        std::vector<std::string> argv;
        for(const auto& s: l) {
            argv.push_back(s);
        }
        runCmd(cmdType, argv);
    }
    void receiveInput(QString input) {
        interpreter->input(input.toStdString());
        oneLineInterpreter->input(input.toStdString());
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
    void receiveAST(std::string ast) {
        emit sendAST(QString::fromStdString(ast));
    }
public:
    CmdExecutor() {
        tokenizer = std::make_shared<Token::Tokenizer>();
        auto symbol_table = std::make_shared<SymbolTable>();
        env = std::make_shared<Env>(symbol_table);
        parser = std::make_shared<Parser>(tokenizer);
        interpreter = std::make_shared<Interpreter>(parser, env);
        auto oneLineTokenizer = std::make_shared<Token::Tokenizer>();
        auto oneLineParser = std::make_shared<Parser>(oneLineTokenizer);
        auto oneLineEnv = std::make_shared<Env>(symbol_table);
        oneLineInterpreter = std::make_shared<Interpreter>(oneLineParser, oneLineEnv);
        oneLineInterpreter->setMode(ProgramMode::NORMAL); // always normal mode
        connect(interpreter->getOutputStream(), &MockOutputStream::sendOutput,
            this, &CmdExecutor::receiveOutput);
        connect(interpreter->getASTStream(), &MockOutputStream::sendOutput,
            this, &CmdExecutor::receiveAST);
        connect(interpreter->getInputStream(), &MockInputStream::waitingForInput,
            this, &CmdExecutor::waitingForInput);
        connect(oneLineInterpreter->getOutputStream(), &MockOutputStream::sendOutput,
    this, &CmdExecutor::receiveOutput);
        // oneLineInterpreter不连接AST输出, 不影响调试器和代码等
        connect(oneLineInterpreter->getInputStream(), &MockInputStream::waitingForInput,
            this, &CmdExecutor::waitingForInput);
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
    void handleAnonymousProgramCmd(std::string cmd) {
        auto parser = oneLineInterpreter->getParser();
        parser->clear();
        auto old_env = oneLineInterpreter->copyEnv();
        oneLineInterpreter->reset();
        auto tokenizer = parser->getTokenizer();
        tokenizer->read_anonymous_line(cmd);
        parser->parseProgram();
        oneLineInterpreter->setEnv(old_env);
        std::string repl = "\n";
        for(const auto& s: oneLineInterpreter->getEnv()->symbol_table->getRepl()) {
            repl += s + "\n";
        }
        print("[DEBUG] one line current env: {}", repl);
        std::cout << fflush;
        oneLineInterpreter->interpret();
        repl = "\n";
        for(const auto& s: oneLineInterpreter->getEnv()->symbol_table->getRepl()) {
            repl += s + "\n";
        }
        print("[DEBUG] one line current env: {}", repl);
        std::cout << fflush;
    }
    bool reloadProgram(const vector<std::string>& new_program_lines) {
        print("[DEBUG] Reloaded program\n");
        bool success = true;
        auto old_program = tokenizer->get_program();
        try {
            interpreter->reload(new_program_lines);
        } catch (std::exception& e) {
            success = false;
            // 异步提示
            interpreter->reload(old_program);
            emit sendError(QString::fromStdString(e.what()));
            print("Failed to reload program: {}\n", e.what());
        }
        return success;
    }
    [[nodiscard]] std::shared_ptr<Parser> getParser() const {
        return parser;
    }
    [[nodiscard]] std::shared_ptr<Env> getEnv() const {
        return env;
    }
    [[nodiscard]] std::shared_ptr<Env> getOneLineEnv() const {
        return oneLineInterpreter->getEnv();
    }
    [[nodiscard]] std::set<int> getBreakpoints() const {
        return interpreter->getStatus().breakpoints;
    }
    [[nodiscard]] ProgramMode getMode() const {
        return mode;
    }
    [[nodiscard]] std::shared_ptr<Interpreter> getInterpreter() const {
        return interpreter;
    }
    [[nodiscard]] std::shared_ptr<Interpreter> getOneLineInterpreter() const {
        return oneLineInterpreter;
    }
    void setMode(ProgramMode m) {
        mode = m;
        interpreter->switchMode(m);
    }
};



#endif //CMD_EXECUTOR_H
